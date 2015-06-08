#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sherlock.h>
#include <delorean.h>
#include <core_cpu.h>
#include <util_hash.h>
#include <gdbstub.h>

//#define DEBUG_GDB_SRV

#define NUM_CORE_REGS 26
#define stl_p(p, v) stl_be_p(p, v)

static int           write_watchpoint = 0;
static unsigned long watchpoint_new_value = 0;
static unsigned long watchpoint_address = 0;

static inline void stl_be_p(void *ptr, int v)
{
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile ("bswap %0\n"
                  "movl %0, %1\n"
                  : "=r" (v)
                  : "m" (*(uint32_t *)ptr), "0" (v));
#else
    uint8_t *d = (uint8_t *) ptr;
    d[0] = v >> 24;
    d[1] = v >> 16;
    d[2] = v >> 8;
    d[3] = v;
#endif
}


enum{
    GDB_SIGNAL_0 = 0,
    GDB_SIGNAL_INT = 2,
    GDB_SIGNAL_TRAP = 5,
    GDB_SIGNAL_UNKNOWN = 143
};

enum
{
    TARGET_SIGINT = 2,
    TARGET_SIGTRAP = 5
};

enum RSState { 
    RS_IDLE,
    RS_GETLINE,
    RS_CHKSUM1,
    RS_CHKSUM2
};

#define GET_REG32(val) do { \
    stl_p(mem_buf, val); \
    return 4; \
    } while(0)

struct GDBState m_gdb;

/**
* @brief 
*
* @param port
*
* @return 
*/
static int gdb_open (int port)
{
    struct sockaddr_in sa;
    int    fd, ret;

    fd = socket (PF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror ("socket");
        return -1;
    }

    /* allow fast reuse */
    int                   val = 1;
    setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof (val));

    sa.sin_family = AF_INET;
    sa.sin_port = htons (port);
    sa.sin_addr.s_addr = 0;
    ret = bind (fd, (struct sockaddr *) &sa, sizeof (sa));
    if (ret < 0)
    {
        perror("bind");
        return -1;
    }

    ret = listen (fd, 0);
    if (ret < 0)
    {
        perror("listen");
        return -1;
    }

    return fd;
}

/**
* @brief 
*/
static void close_gdb_sockets (void)
{
    if (m_gdb.fd)
    {
       close (m_gdb.fd);
       m_gdb.fd = 0;
    }

    if (m_gdb.srv_sock_fd)
    {
        close (m_gdb.srv_sock_fd);
        m_gdb.srv_sock_fd = 0;
    }
}

/**
* @brief 
*
* @param s
*/
static void gdb_accept (struct GDBState *s)
{
    struct sockaddr_in sa;
    socklen_t          len;
    int                fd;

    len = sizeof (sa);
    fd = accept (s->srv_sock_fd, (struct sockaddr *) &sa, &len);
    if (fd < 0 && errno != EINTR)
    {
        perror ("accept");
        return;
    }
    printf ("GDB connected.\n");

    s->fd = fd;
}

/**
* @brief 
*
* @param port
*
* @return 
*/
int gdb_start_and_wait (int port)
{
    m_gdb.running_state = GDB_STATE_DETACH;

    m_gdb.srv_sock_fd = gdb_open(port);
    if (m_gdb.srv_sock_fd < 0){
        printf ("Error: Cannot open port %d in %s\n", port, __FUNCTION__);
        return -1;
    }

    printf ("Waiting for a GDB connection on port %d \n", port);
    gdb_accept (&m_gdb);

    m_gdb.running_state = GDB_STATE_INIT;
    m_gdb.c_cpu_index = -1;
    atexit (close_gdb_sockets);

    return 0;
}

/**
* @brief 
*
* @param v
*
* @return 
*/
static inline int fromhex(int v)
{
    if (v >= '0' && v <= '9')
        return v - '0';
    else if (v >= 'A' && v <= 'F')
        return v - 'A' + 10;
    else if (v >= 'a' && v <= 'f')
        return v - 'a' + 10;
    else
        return 0;
}

/**
* @brief 
*
* @param v
*
* @return 
*/
static inline int tohex(int v)
{
    if (v < 10)
        return v + '0';
    else
        return v - 10 + 'a';
}


/**
* @brief 
*
* @param buf
* @param mem
* @param len
*/
static void memtohex (char *buf, const uint8_t *mem, int len)
{
    int  i, c;
    char *q;

    q = buf;
    for (i = 0; i < len; i++) {
        c = mem[i];
        *q++ = tohex (c >> 4);
        *q++ = tohex (c & 0xf);
    }
    *q = '\0';
}

/**
* @brief 
*
* @param s
*
* @return 
*/
static int get_char (struct GDBState *s)
{
    uint8_t ch;
    int     ret;

    for(;;) {
        ret = recv (s->fd, &ch, 1, 0);
        if (ret < 0) {
            if (errno == ECONNRESET)
                s->fd = -1;
            if (errno != EINTR && errno != EAGAIN)
                return -1;
        } else {
            if (ret == 0) {
                close  (s->fd);
                s->fd = -1;
                return -1;
            }
            break;
        }
    }
    return ch;
}

/**
* @brief 
*
* @param s
* @param buf
* @param len
*/
static void put_buffer (struct GDBState *s, const uint8_t *buf, int len)
{
    int         ret;

    while (len > 0) {
        ret = send (s->fd, buf, len, 0);
        if (ret < 0) {
            if (errno != EINTR && errno != EAGAIN) {
                s->running_state = GDB_STATE_DETACH;
                return;
            }
        } else {
            buf += ret;
            len -= ret;
        }
    }
}


/**
* @brief 
*
* @param s
* @param buf
* @param len
*
* @return 
*/
static int put_packet_binary (struct GDBState *s, const char *buf, int len)
{
    int     csum, i;
    uint8_t *p;

    for (;;) {
        p = s->last_packet;
        *(p++) = '$';
        memcpy (p, buf, len);
        p += len;
        csum = 0;
        for(i = 0; i < len; i++) {
            csum += buf[i];
        }
        *(p++) = '#';
        *(p++) = tohex ((csum >> 4) & 0xf);
        *(p++) = tohex((csum) & 0xf);

        s->last_packet_len = p - s->last_packet;
        put_buffer (s, (uint8_t *) s->last_packet, s->last_packet_len);

        i = get_char (s);
        if (i < 0)
            return -1;
        if (i == '+')
            break;
    }

    return 0;
}

/**
* @brief 
*
* @param s
* @param buf
*
* @return 
*/
static int put_packet (struct GDBState *s, const char *buf)
{
    #ifdef DEBUG_GDB_SRV
    printf ("response:%s\n", buf);
    #endif

    return put_packet_binary (s, buf, strlen(buf));
}

/**
* @brief 
*
* @param cpu
* @param mem_buf
* @param reg
*
* @return 
*/
static int gdb_read_register(cpu_t* cpu, uint8_t *mem_buf, int reg)
{   
    
    if (reg < 16) {
        /* Core integer register.  */
          memcpy((uint8_t*)mem_buf,(uint8_t*)&cpu->regs[reg].data, 4);
          return 4;
    }
    if (reg < 24) {
        /* FPA registers.  */
        memset (mem_buf, 0, 12);
        return 12;
    }

    switch (reg) {
    case 24:
        /* FPA status register.  */
      GET_REG32 (0);
    case 25:
        /* CPSR */
        memcpy((uint8_t*)mem_buf,(uint8_t*)&cpu->cpsr.data, 4);
        return 4;

    default:
    break;
    }

    /* Unknown register.  */
    return 0;
}

/**
* @brief 
*
* @param s
* @param running_state
*/
static void gdb_continue (struct GDBState *s, int running_state)
{
    s->running_state = running_state;
}


/**
* @brief 
*
* @param addr
* @param len
* @param type
*
* @return 
*/
static int gdb_breakpoint_insert (unsigned long addr, unsigned long len, int type)
{
    switch (type)
    {
    case GDB_BREAKPOINT_SW:
    case GDB_BREAKPOINT_HW:
    {
        int             nb;
        unsigned long   *paddr;

        nb = m_gdb.breakpoints.nb;
        paddr = m_gdb.breakpoints.addr;
        paddr[nb] = addr;
        m_gdb.breakpoints.nb++;

        return 0;
    }

    case GDB_WATCHPOINT_WRITE:
    case GDB_WATCHPOINT_READ:
    case GDB_WATCHPOINT_ACCESS:
    {
        int                     nb;
        struct watch_el_t       *pwatch;

        nb = m_gdb.watchpoints.nb;
        pwatch = m_gdb.watchpoints.watch;
        /*
         int                    i;
         for (i = 0; i < nb; i++)
            if (addr == pwatch[i].begin_address)
                break;
        if (i >= nb)
        {*/
            pwatch[nb].begin_address = addr;
            pwatch[nb].end_address = addr + len;
            pwatch[nb].type = type;
            m_gdb.watchpoints.nb++;
        //}
        return 0;
    }

    default:
        return -ENOSYS;
    }
}

/**
* @brief 
*
* @param addr
* @param len
* @param type
*
* @return 
*/
static int gdb_breakpoint_remove (unsigned long addr, unsigned long len, int type)
{
    switch (type)
    {
    case GDB_BREAKPOINT_SW:
    case GDB_BREAKPOINT_HW:
    {
        int                 i, nb;
        unsigned long       *paddr;

        nb = m_gdb.breakpoints.nb;
        paddr = m_gdb.breakpoints.addr;
        for (i = 0; i < nb; i++)
            if (addr == paddr[i])
                break;
        if (i < nb) {
            for ( ;i < nb - 1; i++)
                paddr[i] = paddr[i + 1];
            m_gdb.breakpoints.nb--;
        }
        return 0;
   }

    case GDB_WATCHPOINT_WRITE:
    case GDB_WATCHPOINT_READ:
    case GDB_WATCHPOINT_ACCESS:
    {
        int                     i, nb;
        struct watch_el_t       *pwatch;

        nb = m_gdb.watchpoints.nb;
        pwatch = m_gdb.watchpoints.watch;
        for (i = 0; i < nb; i++)
            if (addr == pwatch[i].begin_address &&
                (addr + len) == pwatch[i].end_address &&
                type == pwatch[i].type)
                break;

        if (i < nb)
        {
            for ( ;i < nb - 1; i++)
                pwatch[i] = pwatch[i + 1];
            m_gdb.watchpoints.nb--;
        }

        return 0;
    }
    default:
        return -ENOSYS;
    }
}

/**
* @brief 
*/
static void gdb_breakpoint_remove_all (void)
{
    m_gdb.breakpoints.nb = 0;
    m_gdb.watchpoints.nb = 0;
}




/**
* @brief 
*
* @param s
* @param line_buf
*
* @return 
*/
static int gdb_handle_packet (struct GDBState *s, const char *line_buf)
{
    const char *p;
    int        ch, type, thread,reg_size, len, res, i;
    char       buf[GDB_MAX_PACKET_LENGTH];
    uint8_t                 mem_buf[GDB_MAX_PACKET_LENGTH];
//    uint8_t                 *registers;
    unsigned long           addr;
    uint32_t       *ptr_buf;
//    void                    *hw_addr;

    #ifdef DEBUG_GDB_SRV
    printf ("command='%s'\n", line_buf);
    #endif

    p = line_buf;
    ch = *p++;
    switch(ch) {
    case '?': //reason the target halted
        snprintf (buf, sizeof (buf), "T%02xthread:%x;",
        GDB_SIGNAL_TRAP, s->cpu->id+1);
        put_packet (s, buf);
        /* Remove all the breakpoints when this query is issued,
         * because gdb is doing and initial connect and the state
         * should be cleaned up.
         */
        gdb_breakpoint_remove_all ();
        break;
    case 'c': //caddr - continue
//      if (*p != '\0') {
//          addr = strtoull (p, (char **) &p, 16);
//          /*
//          #if defined (TARGET_ARM)
//          s->c_cpu->regs[15] = addr;
//          #endif
//          */
//      }
        gdb_continue (s, GDB_STATE_CONTINUE);
        break;
    case 'C': // Csig;addr - continue with signal
        gdb_continue (s, GDB_STATE_CONTINUE);
        break;
    case 'D': //D - detach
//        gdb_breakpoint_remove_all ();
        gdb_continue (s, GDB_STATE_DETACH);
        put_packet (s, "OK");
        break;

    case 'g': //read registers
        len = 0;
        for (addr = 0; addr < NUM_CORE_REGS; addr++) {
            reg_size = gdb_read_register(s->cpu, mem_buf + len, addr);
            len += reg_size;
        }
        memtohex (buf, mem_buf, len);
        put_packet (s, buf);
        break;
    case 'G': //GXX... - write regs
//      registers = mem_buf;
//      len = strlen(p) / 2;
//      hextomem ((uint8_t *) registers, p, len);
//
//      for (addr = 0; addr < num_g_regs && len > 0; addr++)
//      {
//          reg_size = gdb_write_register (((CPUState **) crt_qemu_instance->m_envs)[s->g_cpu_index],
//              registers, addr);
//          len -= reg_size;
//          registers += reg_size;
//      }
//      put_packet(s, "OK");
        break;
    case 'H': //set thread
        type = *p++;
        thread = strtoull (p, (char **) &p, 16);
        if (thread == -1 || thread == 0) {
            if (type == 'c')
                s->c_cpu_index = -1;
            put_packet (s, "OK");
            break;
        }
        switch (type) {
        case 'c':
            s->c_cpu_index = thread - 1;
            put_packet (s, "OK");
            break;
        case 'g':
            s->g_cpu_index = thread - 1;
            put_packet (s, "OK");
            break;
        default:
             put_packet (s, "E22");
             break;
        }
        break;
    case 'k': //k - kill request
        fprintf (stderr, "\nTerminated via GDB!\n");
        exit (0);
    case 'm': //read memory
        addr = strtoull (p, (char **) &p, 16);
//        addr = get_phys_addr_gdb (addr);
        if (*p == ',')
            p++;

        len = strtoull (p, NULL, 16);
      if (write_watchpoint && addr == watchpoint_address){
//            hw_addr = &watchpoint_new_value;
            ptr_buf = (uint32_t*)mem_buf;
            *ptr_buf = watchpoint_new_value;
        } else{
            ptr_buf = (uint32_t*)mem_buf;
            for(i = 0; i < (len>>2 ? len>>2 : 1); i++, addr+=4){
                ptr_buf[i] = mem_get_addr_value(addr);
            }
      }
//        if (hw_addr == NULL) { // Out of range test
//            put_packet (s, "E14");
//            break;
//        }

//        memcpy (mem_buf, hw_addr, len); // TODO: use mem_buf instead of hw_addr above

        memtohex (buf, mem_buf, len);
        put_packet (s, buf);

        break;
    case 'M': //write memory
//        addr = strtoull (p, (char **) &p, 16);
//        if (*p == ',')
//            p++;
//        len = strtoull (p, (char **) &p, 16);
//        if (*p == ':')
//            p++;
//        hextomem (mem_buf, p, len);
//
//        hw_addr = (uint8_t *)crt_qemu_instance->m_systemc.systemc_get_mem_addr (
//             cpu_single_env->rabbits.sc_obj,
//             crt_qemu_instance->m_systemc.subsystem,
//             addr);
//        if (hw_addr == NULL){
//            put_packet (s, "E14");
//            break;
//        }
//        memcpy (hw_addr, mem_buf, len);
//
//        put_packet (s, "OK");
        break;

    case 'p': //pn - read reg
        /* Older gdb are really dumb, and don't use 'g' if 'p' is avaialable.
           This works, but can be very slow.  Anything new enough to
           understand XML also knows how to use this properly.  */
//        addr = strtoull (p, (char **) &p, 16);
//
//        reg_size = gdb_read_register (cpu_single_env, mem_buf, addr);
//       if (reg_size) {
//            memtohex (buf, mem_buf, reg_size);
//            put_packet (s, buf);
//        } else {
//            put_packet(s, "E14");
//        }
        break;

    case 'P': //Pn...=r... - write reg
//        addr = strtoull (p, (char **) &p, 16);
//        if (*p == '=')
//            p++;
//        reg_size = strlen(p) / 2;
//        hextomem (mem_buf, p, reg_size);
//        gdb_write_register (cpu_single_env, mem_buf, addr);
//        put_packet(s, "OK");
        break;

    case 'q': //general query
    case 'Q':
        if (strcmp(p, "C") == 0) {
            sprintf (buf, "QC%x", 1);
            put_packet (s, buf);
            break;
        }
        else if (strncmp(p, "Offsets", 7) == 0) {
            snprintf (buf, sizeof(buf), "Text=%x;Data=%x;Bss=%x",
                0x00000000, 0x00000000, 0x00000000);
            put_packet (s, buf);
            break;
        } else if (strncmp(p, "Supported", 9) == 0) {
            snprintf (buf, sizeof (buf), "PacketSize=%x;ReverseContinue+;ReverseStep+", GDB_MAX_PACKET_LENGTH);
            put_packet (s, buf);
            break;
        } else if (strcmp (p,"fThreadInfo") == 0) {
            s->query_cpu_index = 0;
            goto report_cpuinfo;
        } else if (strcmp (p,"sThreadInfo") == 0) {
            report_cpuinfo:
            if (s->query_cpu_index < oracle_get_ncpu()/*crt_qemu_instance->m_NOCPUs*/) {
                snprintf (buf, sizeof (buf), "m%x", s->query_cpu_index + 1);
              put_packet (s, buf);
              s->query_cpu_index++;
            } else
                put_packet (s, "l");
            break;
        } else if (strncmp(p,"ThreadExtraInfo,", 16) == 0) {
            thread = strtoull (p + 16, (char **) &p, 16);
            if (thread > oracle_get_ncpu()/*crt_qemu_instance->m_NOCPUs*/)
                put_packet(s, "E14");
            else {
                len = snprintf ((char *) mem_buf, sizeof (mem_buf),
                    "CPU %d [%s]", thread, m_gdb.state ? "halted " : "running"); // TODO redo it
                memtohex (buf, mem_buf, len);
                put_packet (s, buf);
            }
         
            break;
        }
        goto unknown_command;

    case 's': //saddr - step
        
        if (*p != '\0') {
            addr = strtoull (p, (char **) &p, 16);
            /*
            #if defined (TARGET_ARM)
            s->c_cpu->regs[15] = addr;
            #endif
            */
        }        
        gdb_continue (s, GDB_STATE_STEP);
        break;
    case 'T': //TXX - thread alive
        thread = strtoull (p, (char **) &p, 16);
        if (thread > 0 && thread <= oracle_get_ncpu())
             put_packet(s, "OK");
        else
            put_packet(s, "E22");
        break;
    case 'v':
        if (!strncmp (p, "Cont", 4)) {
            p += 4;
            if (!strcmp (p, "?"))
                put_packet (s, "vCont;c;C;s;S");
            else if (*p == ';') {
                p++;
                if (*p == 'c') {
                    p++;
                    if (*p == ':') {
//                        one_cpu = 1;
//                        saved_c_cpu_index = s->c_cpu_index;
                        thread = strtoull (p + 1, (char **) &p, 16) - 1;
                        s->c_cpu_index = thread;
                    }

                    gdb_continue (s, GDB_STATE_CONTINUE);
                } else if (*p == 's') {
                    p++;
                    if (*p == ':') {
                      if (write_watchpoint == 1) {
                            put_packet (s, "S05");
//                            gdb_continue (s, GDB_STATE_STEP);

                            break;
                        } else {
//                          one_cpu = 1;
//                            saved_c_cpu_index = s->c_cpu_index;
                              thread = strtoull (p + 1, (char **) &p, 16) - 1;
                              s->c_cpu_index = thread;
                        }
                    }
                    gdb_continue (s, GDB_STATE_STEP);
                } else
                    goto unknown_command;
            }
        }
        break;
    case 'b':
        if (!strncmp (p, "c", 1)) {
//            put_packet(s, "OK");
            gdb_continue (s, GDB_STATE_REVERSE_CONTINUE);
        } else if (*p == 's'){
//            put_packet(s, "OK");
            gdb_continue (s, GDB_STATE_REVERSE_STEP);
        } else goto unknown_command;
    break; 

    case 'Z': //zt,addr,length - add break or watchpoint 
    case 'z': //zt,addr,length - remove break or watchpoint 
      type = strtoul (p, (char **) &p, 16);
      if (*p == ',')
          p++;
      addr = strtoull (p, (char **) &p, 16);
      if (*p == ',')
          p++;
      len = strtoull (p, (char **) &p, 16);
      res = 0;
      //if (addr > 0x10020)
      {
          if (ch == 'Z')
            res = gdb_breakpoint_insert (addr, len, type);
        else
            res = gdb_breakpoint_remove (addr, len, type);
      }
      res =1; 

      if (res >= 0)
           put_packet(s, "OK");
      else
          if (res == -ENOSYS)
              put_packet(s, "");
          else
              put_packet(s, "E22");
        break;

    default:
    unknown_command:
        /* put empty packet */
        buf[0] = '\0';
        put_packet (s, buf);
        break;
    }

    return RS_IDLE;
}

/**
* @brief 
*
* @param s
* @param ch
*/
static void gdb_read_byte (struct GDBState *s, int ch)
{
    uint32_t i, csum;
    uint8_t  reply;

    switch (s->state){
    case RS_IDLE:
        if (ch == '$'){
            s->line_buf_index = 0;
            s->state = RS_GETLINE;
        }
        break;
    case RS_GETLINE:
        if (ch == '#'){
            s->state = RS_CHKSUM1;
        } else {
            if (s->line_buf_index >= sizeof(s->line_buf) - 1) {
                s->state = RS_IDLE;
            } else {
                s->line_buf[s->line_buf_index++] = ch;
            }
        }
        break;
    case RS_CHKSUM1:
        s->line_buf[s->line_buf_index] = '\0';
        s->line_csum = fromhex(ch) << 4;
        s->state = RS_CHKSUM2;
        break;
    case RS_CHKSUM2:
        s->line_csum |= fromhex(ch);
        csum = 0;
        for(i = 0; i < s->line_buf_index; i++) {
            csum += s->line_buf[i];
        }
        if (s->line_csum != (csum & 0xff)) {
            reply = '-';
            put_buffer (s, &reply, 1);
            s->state = RS_IDLE;
        } else {
            reply = '+';
            put_buffer (s, &reply, 1);
            s->state = gdb_handle_packet (s, s->line_buf);
        }
        break;
    }          
}

/**
 * @brief 
 *
 * @param idx_watch
 * @param bwrite
 * @param new_val
 * @param cpu_i
 */
void gdb_loop(int idx_watch, int bwrite, unsigned long new_val, uint16_t cpu_i)
{
    char            buf[256];
    char            buf1[256];
    int             i, nb;
    struct GDBState *s = &m_gdb;

    write_watchpoint = 0;
    s->cpu = oracle_get_cpu(cpu_i);
    
    if (s->running_state != GDB_STATE_INIT){
        sprintf (buf, "T%02x", TARGET_SIGTRAP);

        {
        char          reg[4][20];
        unsigned long ul;
        memtohex (reg[0], (uint8_t*) &s->cpu->regs[15].data, 4); // pc
        memtohex (reg[1], (uint8_t*) &s->cpu->regs[13].data, 4); //sp
        ul = s->cpu->cpsr.data;
        memtohex (reg[2], (uint8_t*) &ul, 4); //cpsr
        memtohex (reg[3], (uint8_t*) &s->cpu->regs[14].data, 4); //lr

        sprintf (buf1, "0f:%s;0d:%s;19:%s;0e:%s;",
            reg[0], reg[1], reg[2], reg[3]);
        strcat (buf, buf1);
        }

        if (idx_watch != -1) {
            struct watch_el_t   *pwatch;
            static const char   *watch_names[] = {"watch", "rwatch", "awatch"};

            pwatch = &s->watchpoints.watch[idx_watch];
            sprintf (buf1, "%s:%lx;",
                watch_names[pwatch->type - GDB_WATCHPOINT_WRITE],
                pwatch->begin_address);
            strcat (buf, buf1);

            if (bwrite)
            {
                write_watchpoint = 1;
                watchpoint_new_value = new_val;
                watchpoint_address = pwatch->begin_address;
            }
        }


        sprintf (buf1, "thread:%x;", cpu_i+1);
        strcat (buf, buf1);

        put_packet (s, buf);

    }


    if (s->running_state == GDB_STATE_DETACH) {
       return;
    }
   
    s->g_cpu_index = cpu_i;
    s->state = RS_IDLE;
    s->running_state = GDB_STATE_CONTROL;
    while(s->running_state == GDB_STATE_CONTROL){
        nb = read (s->fd, buf, 256);      
        
        if( nb > 0 ){
            for(i=0; i < nb; i++)
               gdb_read_byte(s, buf[i]);
        } else if(nb == 0 || errno != EAGAIN){
            printf("GDB Disconnected\n");
            s->running_state = GDB_STATE_DETACH;
            break;
        }
    }
}

/**
* @brief 
*
* @param addr
*
* @return 
*/
int gdb_condition(uint32_t addr)
{
    uint32_t  running_state = m_gdb.running_state;
    uint32_t  i, nb;
    unsigned long  *paddr;
    if(running_state == GDB_STATE_DETACH)
        return 0;
    
    if (running_state == GDB_STATE_STEP || 
        running_state == GDB_STATE_INIT ||
        running_state == GDB_STATE_REVERSE_STEP)
        return 1;
    
    nb = m_gdb.breakpoints.nb;
    paddr = m_gdb.breakpoints.addr;
    for (i = 0; i < nb; i++)
        if (addr == paddr[i])
            return 2;

    return 0;
}

/**
 * @brief 
 *
 * @return 0 for a forward execution or 1 to reverse execution
 */
int gdb_exec_direction(void)
{
    int running_state = m_gdb.running_state;

    if (running_state == GDB_STATE_REVERSE_CONTINUE ||
        running_state == GDB_STATE_REVERSE_STEP){
       return 1;
    }

    return 0;
}

/**
 * @brief 
 *
 * @param comp the component id
 * @param val
 */
void gdb_verify(comp_type_t type_i, gdb_param_t *param)
{ 
    comp_type_t type = type_i;

//    printf("TYPE: %s\n", type == COMP_CPU ? "COMP_CPU" : "COMP_MEM");
    do{
        if(gdb_exec_direction()){// REVERSE
            oracle_reverse_exec(param,&type);
        } else {                 // FORWARD
            if (!oracle_get_trace()){
                oracle_forward_exec(param, &type);
            }
        }

        // GDB STATE MACHINE 
        switch(type){
        case COMP_CPU:
        {
            uint8_t res;
            res = gdb_condition (param->cpu.pc);
            if (res){ // INSTRUCTION
                if(res == 2)
                      output_partial_results();
                gdb_loop(-1, 0, 0, param->cpu.id);
            }
        }
            break;
        case COMP_MEM:
        {
            int i, n = m_gdb.watchpoints.nb;
            struct watch_el_t *pwatch = m_gdb.watchpoints.watch;
            uint32_t addr = param->mem.addr;
            for (i = 0; i < n; i++){
                if (addr >= pwatch[i].begin_address && addr < pwatch[i].end_address &&
                (pwatch[i].type == GDB_WATCHPOINT_WRITE || 
                 pwatch[i].type == GDB_WATCHPOINT_ACCESS)){
                    gdb_loop(i, 1,param->mem.val , param->mem.ref);
                    break;
                }
            }
        }
//            gdb_loop(i, 0, 0, comp_id); // READ
            break;
        default:
            break;
        }
    } while(gdb_exec_direction() || !oracle_get_trace()); 
}

