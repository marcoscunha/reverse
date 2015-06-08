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

#ifndef __BUNNY_H__
#define __BUNNY_H__

#include <system_init.h>

#include <qemu_wrapper.h>
#include <sl_timer_device.h>
#include <sl_tty_device.h>
#include <sl_mailbox_device.h>
#include <framebuffer_device.h>
#include <sl_block_device.h>
#include <aicu_device.h>
#include <mem_device.h>
#include <generic_SMSMP_subsystem.h>


class bunny : public generic_SMSMP_subsystem {

public:

    int                  cpu_nb;
    int                  gdb_port;

private:
    qemu_wrapper      *m_qemu;
    mem_device        *m_ram;
    sl_block_device   *m_bl;
    fb_device         *m_fb;
    sl_tty_device     *m_tty0;
    sl_tty_device     *m_ctrl0;
    sl_mailbox_device *m_mb;
    aicu_device       *m_icu;
    sl_timer_device  **m_timers;

    char              *m_image_fname;

    sc_signal<bool>  *m_aicu_irq;
    sc_signal<bool>  *m_wires_irq_qemu;
    int              *m_int_cpu_mask;

public:
	bunny(sc_module_name _name, init_struct *is);
	~bunny();

    void end_of_elaboration(void);

};

#endif /* __BUNNY_H__ */

/*
 * Vim standard variables
 * vim:set sw=4 ts=4 expandtab tw=80 cindent syntax=c:
 *
 * Emacs standard variables
 * Local Variables:
 * mode: c
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
