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

#ifndef _SL_TTY_DEVICE_H_
#define _SL_TTY_DEVICE_H_

#include <slave_device.h>


#define READ_BUF_SIZE           256

typedef struct
{
    unsigned long       int_enabled;
    unsigned long       int_level;
    uint8_t             read_buf[READ_BUF_SIZE];
    int                 read_pos;
    int                 read_count;
    int                 read_trigger;
} tty_state;

enum sl_tty_registers {
    TTY_WRITE  = 0,
    TTY_STATUS = 1,
    TTY_READ   = 2,
    /**/
    TTY_SPAN   = 4,
};

class sl_tty_device : public slave_device
{
public:
    SC_HAS_PROCESS (sl_tty_device);
    sl_tty_device (sc_module_name _name, int ntty, int hide_xterm=0);
    virtual ~sl_tty_device ();

public:
    /*
     *   Obtained from father
     *   void send_rsp (bool bErr);
     */
    virtual void rcv_rqst (uint32_t ofs, uint8_t be,
                           uint8_t *data, bool bWrite);

private:
    void write (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr);
    void read  (uint32_t ofs, uint8_t be, uint8_t *data, bool &bErr);

    //void irq_update ();
    void read_thread ();
    void irq_update_thread();

public:
    //ports
    sc_out<bool>        irq;

private:
    sc_event            evRead;
    sc_event            irq_update;

    int                 nb_tty;
    int                *m_pin;
    int                *m_pout;
    tty_state           state;
};

#endif

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
