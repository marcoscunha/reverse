/*
 *  Copyright (c) 2010 TIMA Laboratory
 *
 *  This file is part of Rabbits.
 *
 *  Rabbits is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Rabbits is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Rabbits.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sl_tty_device.h>
#include <cfg.h>

#if 0
#define DEBUG_DEVICE_SL_TTY
#endif

#ifdef DEBUG_DEVICE_SL_TTY
#define DPRINTF(fmt, args...)                                           \
    do { fprintf(stderr, "DEBUG sl_tty_device: " fmt , ##args); } while (0)
#define DCOUT if (1) cout
#else
#define DPRINTF(fmt, args...) do {} while(0)
#define DCOUT if (0) cout
#endif

#define TTY_INT_READ        1

static int      s_pid_tty[256]; // 2 per mvep instance
static int      s_nb_tty = 0;

static void close_ttys ()
{
    int         i, status;

    if (s_nb_tty == 0)
        return;

    for (i = 0; i < s_nb_tty; i++)
    {
        kill (s_pid_tty[i], SIGKILL);
        ::wait (&status);
    }

    s_nb_tty = 0;
}

static void sig_hup (int)
{
    exit (1);
}

void sl_tty_device::read_thread ()
{
    fd_set              rfds;
    int                 i;
    int                 pos, ret, nfds = 0;
    uint8_t             ch;
    struct timeval      tv;

    FD_ZERO(&rfds);

    for(i = 0; i < nb_tty; i++){
        nfds = (m_pin[i] > nfds) ? m_pin[i] : nfds;
    }
    nfds++;

    while (1)
    {
        wait (10, SC_US);

        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO (&rfds);
        for(i = 0; i < nb_tty; i++) {
            if(m_pin[i] != -1){
                FD_SET (m_pin[i], &rfds);
            }
        }
        ret = select (nfds, &rfds, NULL, NULL, &tv);
        if (ret > 0) 
        {
            for(i = 0; i < nb_tty; i++) {

                if (FD_ISSET (m_pin[i], &rfds))
                {
                    ::read (m_pin[i], &ch, 1);
                    if(ch >= 0x20 && ch < 0x7F)
                        DPRINTF ("%s read on %s%d : 0x%02x, '%2c'\n",
                                 __FUNCTION__, name(), i, (unsigned int) ch, (char)ch);
                    else
                        DPRINTF ("%s read on %s%d : 0x%02x, XX\n",
                                 __FUNCTION__, name(), i, (unsigned int) ch);

                    while (state.read_count == READ_BUF_SIZE)
                        wait (evRead);
                    
                    pos = (state.read_pos + state.read_count) % READ_BUF_SIZE;
                    state.read_buf[pos] = ch;
                    state.read_count++;
                    
                    state.int_level |= TTY_INT_READ;
                    irq_update.notify ();
                }
            }
        }
    }
    
}

sl_tty_device::sl_tty_device (sc_module_name _name, int ntty, int hide_xterm):
slave_device (_name)
{
    int             i;
    int             ppout[2], ppin[2];
    char            spipeout[16], spipein[16], slog[50], sname[50];

    memset (&state, 0, sizeof (state));
    nb_tty = ntty;
    m_pout = new int[nb_tty];
    m_pin  = NULL;

    if(hide_xterm){

        for (i = 0; i < nb_tty; i++){
            
            sprintf (slog, "%s%02d", (const char *)_name, i);
            m_pout[i] = open(slog, O_CREAT|O_WRONLY, (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));

        }

    }else{

        signal (SIGHUP, sig_hup);
        signal (SIGPIPE, SIG_IGN);
        atexit (close_ttys);

        m_pin  = new int[nb_tty];
        state.int_enabled = 1;

        for (i = 0; i < nb_tty; i++)
        {
            if (pipe (ppout) < 0)
            {
                cerr << name () << " can't open out pipe!" << endl;
                exit (0);
            }
            if (pipe (ppin) < 0)
            {
                cerr << name () << " can't open in pipe!" << endl;
                exit (0);
            }
        
            m_pout[i] = ppout[1];
            sprintf (spipeout, "%d", ppout[0]);
            m_pin[i] = ppin[0];
            sprintf (spipein, "%d", ppin[1]);
            
            if (!(s_pid_tty[s_nb_tty++] = fork ()))
            {
                setpgrp();
                
                sprintf (slog, "%s%02d", (const char *)name(), i);
                sprintf (sname, "%s %d", (const char *)name(), i);
            
                if (execlp ("xterm",
                            "xterm",
                            "-sb","-sl","1000",
                            "-l", "-lf", slog,
                            "-n", sname, "-T", sname,
                            "-class", "RabbitsOUT",
                            "-e", "tty_term_rw", spipeout, spipein,
                            NULL) == -1)
                {
                    perror ("tty_term: execlp failed!");
                    _exit(1);
                }
            }
        }
        SC_THREAD (read_thread);
    }

    SC_THREAD (irq_update_thread);

}

sl_tty_device::~sl_tty_device ()
{
    int             i;
    for (i = 0; i < nb_tty; i++)
        close (m_pout[i]);
    close_ttys ();
    
    delete [] m_pout;
    if(m_pin)
        delete [] m_pin;
    
}

void sl_tty_device::irq_update_thread ()
{
    unsigned long       flags;

    while(1) {

        wait(irq_update);

        flags = state.int_level & state.int_enabled;

        DPRINTF ("%s - %s\n", __FUNCTION__, (flags != 0) ? "1" : "0");
        
        irq = (flags != 0);
    }
}

void sl_tty_device::write (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr)
{
    uint8_t                 tty;
    uint32_t                value;

    bErr = false;

    ofs >>= 2;
    if (be & 0xF0)
    {
        ofs++;
        value = * ((uint32_t *) data + 1);
    }
    else
        value = * ((uint32_t *) data + 0);

    tty = ofs / TTY_SPAN;
    ofs = ofs % TTY_SPAN;

    if(tty >= nb_tty){
        DPRINTF("(TTY too high) Bad %s::%s tty=%d ofs=0x%X, be=0x%X, data=0x%X-%X!\n",
                name (), __FUNCTION__, tty, (unsigned int) ofs, (unsigned int) be,
                (unsigned int) *((uint32_t *)data + 0), (unsigned int) *((uint32_t *)data + 1));
        exit (1);
    }

    switch(ofs)
    {
    case 0: //TTY_WRITE
//        DPRINTF("TTY_WRITE[%d]: 0x%x (%c)\n", tty, (char)value, (char) value);
        ::write (m_pout[tty], &value, 1);
        break;
        

    default:
        printf ("Bad %s::%s ofs=0x%X, be=0x%X, data=0x%X-%X!\n",
                name (), __FUNCTION__, (unsigned int) ofs, (unsigned int) be,
                (unsigned int) *((uint32_t *)data + 0), (unsigned int) *((uint32_t *)data + 1));
        exit (1);
    }
    bErr = false;
}

void sl_tty_device::read (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr)
{
    uint32_t *pdata;
    int      tty;
    char     c;

    pdata = (uint32_t *) data;
    pdata[0] = 0;
    pdata[1] = 0;

    ofs >>= 2;
    if (be & 0xF0)
    {
        ofs++;
        pdata++;
    }

    tty = ofs / TTY_SPAN;
    ofs = ofs % TTY_SPAN;

    if(tty >= nb_tty){
        DPRINTF("(TTY too high) Bad %s::%s tty=%d ofs=0x%X, be=0x%X, data=0x%X-%X!\n",
                name (), __FUNCTION__, tty, (unsigned int) ofs, (unsigned int) be,
                (unsigned int) pdata[0], (unsigned int) pdata[1]);
        exit (1);
    }

    switch (ofs)
    {
    case 1: //TTY_STATUS
        DPRINTF("TTY_STATUS[%s%d]: 0x%x\n", name(), tty, (unsigned int)state.read_count);
        *pdata = state.read_count;

        break;

    case 2: //TTY_READ
        c = state.read_buf[state.read_pos];
        DPRINTF("TTY_READ[%s%d]: 0x%x (%c)\n", name(), tty, (char)c, (char)c);
        if (state.read_count > 0)
        {
            state.read_count--;
            if (++state.read_pos == READ_BUF_SIZE)
                state.read_pos = 0;

            if (0 == state.read_count)
            {
                state.int_level &= ~TTY_INT_READ;
                irq_update.notify ();
            }

            evRead.notify (0, SC_NS);
        }
        *pdata = c;
        break;

    case 0: //TTY_WRITE
    default:
        printf ("Bad %s::%s ofs=0x%X, be=0x%X!\n",
                name (), __FUNCTION__, (unsigned int) ofs, (unsigned int) be);
        exit (1);
    }
    bErr = false;
}

void sl_tty_device::rcv_rqst (uint32_t ofs, uint8_t be,
                                  uint8_t *data, bool bWrite)
{

    bool bErr = false;

    if (bWrite)
        this->write(ofs, be, data, bErr);
    else
        this->read(ofs, be, data, bErr);

    send_rsp (bErr);
}

/*
 * Vim standard variables
 * vim:set ts=4 expandtab tw=80 cindent syntax=c:
 *
 * Emacs standard variables
 * Local Variables:
 * mode: c
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
