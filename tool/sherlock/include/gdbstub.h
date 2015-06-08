#ifndef _GDBSTUB_H_
#define _GDBSTUB_H_

#define GDB_MAX_PACKET_LENGTH (4096 * 4)

enum        
{       
    GDB_STATE_CONTROL,
    GDB_STATE_STEP,
    GDB_STATE_CONTINUE,
    GDB_STATE_DETACH,
    GDB_STATE_INIT,
    GDB_STATE_REVERSE_STEP,
    GDB_STATE_REVERSE_CONTINUE
};  

enum
{
    GDB_BREAKPOINT_SW,
    GDB_BREAKPOINT_HW,
    GDB_WATCHPOINT_WRITE,
    GDB_WATCHPOINT_READ,
    GDB_WATCHPOINT_ACCESS
};

typedef struct gdb_cpu{
    uint16_t  id;
    uint32_t pc;
}gdb_cpu_t;

typedef struct gdb_mem{
    uint16_t id;
    uint16_t ref;
    uint32_t addr;
    uint32_t val;
}gdb_mem_t;

typedef union gdb_param{
    gdb_cpu_t cpu;
    gdb_mem_t mem;
}gdb_param_t;

struct GDBState
{
    int                 fd;
    int                 srv_sock_fd;
    int                 c_cpu_index;
    int                 g_cpu_index;
    int                 query_cpu_index;
    int                 state;  /* parsing state */
    int                 running_state;
    char                line_buf[GDB_MAX_PACKET_LENGTH];
    int                 line_buf_index;
    int                 line_csum;
    uint8_t             last_packet[GDB_MAX_PACKET_LENGTH + 4];
    int                 last_packet_len;
    
    cpu_t               *cpu;

    struct breakpoint_t     
    {
        unsigned long   addr[100];
        int             nb; 
    } breakpoints;          
    struct watchpoint_t     
    {
        struct watch_el_t
        {
            unsigned long   begin_address;
            unsigned long   end_address;
            int             type;
        } watch [100];
        int             nb;
    } watchpoints;
    
};  

void gdb_verify(comp_type_t type, gdb_param_t *param);
void gdb_process(cpu_t* cpu, unsigned long pc);
int  gdb_start_and_wait (int port);


#endif
