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
#include <app_loader.h>

#include <bunny.h>

/*
 * for QEMU_ADDR_BASE and 
 *   SET_SYSTEMC_INT_ENABLE
 */
#include <qemu_wrapper_cts.h>

/*
 * Address Mapping
 */
interconnect_address_map_elt_t addr_map_elt[] =
{
    { 0 , 0x00000000, 0x40000000 }, // 1 Gbyte RAM    { 0 , 0x00000000, 0x20000000 }, // 512 Mbytes RAM
    { 1 , 0xC1000000, 0xC1000020 },
    { 2 , 0xC3000000, 0xC3000100 },
    { 3 , 0xC4000000, 0xC4000200 },
    { 4 , 0xC5000000, 0xC5100000 },
    { 5 , 0xC6000000, 0xC6100000 },
    { 6 , 0xC7000000, 0xC7000010 },
    { 7 , 0xC2000000, 0xC2000010 },
    { 8 , 0xC2000010, 0xC2000020 },
    { 9 , 0xC2000020, 0xC2000030 },
    { 10, 0xC2000030, 0xC2000040 },
    { 11, 0xC2000040, 0xC2000050 },
    { 12, 0xC2000050, 0xC2000060 },
    { 13, 0xC2000060, 0xC2000070 },
    { 14, 0xC2000070, 0xC2000080 },
    { 15, 0xC2000080, 0xC2000090 },
    { 16, 0xC2000090, 0xC20000a0 },
    { 17, 0xC20000a0, 0xC20000b0 },
    { 18, 0xC20000b0, 0xC20000c0 },
    { 19, 0xC20000c0, 0xC20000d0 },
    { 20, 0xC20000d0, 0xC20000e0 },
    { 21, 0xC20000e0, 0xC20000f0 },
    { 22, 0xC20000f0, 0xC2000100 },
    { 23, 0xC2000100, 0xC2000110 },
    { 24, 0xC2000110, 0xC2000120 },
    { 25, 0xC2000120, 0xC2000130 },
    { 26, 0xC2000130, 0xC2000140 },
    { 27, 0xC2000140, 0xC2000150 },
    { 28, 0xC2000150, 0xC2000160 },
    { 29, 0xC2000160, 0xC2000170 },
    
};

interconnect_address_map_t addr_map = 
{
    30, /* .n_elts = */
    addr_map_elt,
};

/*
 *
 *
 */

bunny::bunny(sc_module_name _name, init_struct *is)
:generic_SMSMP_subsystem(_name, 50, 50)
{
    int                i, len;
    int                n_mb  = 0;
    fb_reset_t         fb_res_stat;

    cpu_nb = is->no_cpus;
    n_mb  = is->no_cpus;

    len = strlen(is->kernel_filename) + 1;
    m_image_fname = new char[len];
    strcpy(m_image_fname, is->kernel_filename);

    if (is->fb_uninit){
      fb_res_stat.fb_start  = 0;
      fb_res_stat.fb_w      = 0;
      fb_res_stat.fb_h      = 0;
      fb_res_stat.fb_mode   = NONE;
      fb_res_stat.fb_display_on_wrap = 0;
    }else{
      fb_res_stat.fb_start  = 1;
      fb_res_stat.fb_w      = 256;
      fb_res_stat.fb_h      = 144;
      fb_res_stat.fb_mode   = YV16;
      fb_res_stat.fb_display_on_wrap = 1;
    }

    //slaves
    m_ram   = new mem_device ("dynamic", is->ramsize + 0x1000,
                            get_subsystem());
    m_bl    = new sl_block_device("block", is->block_device, 1024);
    m_fb    = new fb_device("fb", &fb_res_stat);
    m_tty0  = new sl_tty_device ("tty", 1);
    m_ctrl0 = new sl_tty_device ("ctrl", 1, 1);
    m_mb    = new sl_mailbox_device("mb", n_mb);
    m_icu   = new aicu_device("aicu", is->no_cpus, 4, 2);
    m_timers = new sl_timer_device *[is->no_cpus];

    push_slave(m_ram->get_slave());    // 0
    push_slave(m_tty0->get_slave());   // 1
    push_slave(m_mb->get_slave());     // 2
    push_slave(m_icu->get_slave());    // 3
    push_slave(m_fb->get_slave());     // 4
    push_slave(m_bl->get_slave());     // 5
    push_slave(m_ctrl0->get_slave());  // 6

    for (i = 0; i < is->no_cpus; i++){
        char        buf[20];
        sprintf (buf, "timer_%d", i);
        m_timers[i] = new sl_timer_device (buf);
        push_slave(m_timers[i]->get_slave()); // 7 + i
    };

    // Connecting Qemu to AICU (IRQ wires);
    m_int_cpu_mask = new int [is->no_cpus];
    m_wires_irq_qemu = new sc_signal<bool>[is->no_cpus];

    for(i = 0; i < is->no_cpus; i++){
        m_int_cpu_mask[i] = (1<<i);
        m_icu->irq_out[i](m_wires_irq_qemu[i]);
    }

    // Connecting AICU irq wires to peripherals
    m_aicu_irq = new sc_signal<bool> [2*is->no_cpus + 4];
    int aicu_irq_ind = 0;

    for(i = 0; i < is->no_cpus; i++){
        fprintf(stderr, "MB%d on AICU_in%d\n", i, aicu_irq_ind);
        m_mb->irq[i] (m_aicu_irq[aicu_irq_ind]);
        m_icu->irq_in[aicu_irq_ind](m_aicu_irq[aicu_irq_ind]);
        aicu_irq_ind++;
    }

    for(i = 0; i < is->no_cpus; i++){
        fprintf(stderr, "Timer%d on AICU_in%d\n", i, aicu_irq_ind);
        m_timers[i]->irq (m_aicu_irq[aicu_irq_ind]);
        m_icu->irq_in[aicu_irq_ind](m_aicu_irq[aicu_irq_ind]);
        aicu_irq_ind++;
    }

    fprintf(stderr, "TTY on AICU_in%d\n", aicu_irq_ind);
    m_tty0->irq(m_aicu_irq[aicu_irq_ind]);
    m_icu->irq_in[aicu_irq_ind](m_aicu_irq[aicu_irq_ind]);
    aicu_irq_ind++;

    fprintf(stderr, "CTRL on AICU_in%d\n", aicu_irq_ind);
    m_ctrl0->irq(m_aicu_irq[aicu_irq_ind]);
    m_icu->irq_in[aicu_irq_ind](m_aicu_irq[aicu_irq_ind]);
    aicu_irq_ind++;
 
    fprintf(stderr, "BlockDevice on AICU_in%d\n", aicu_irq_ind);
    m_bl->irq (m_aicu_irq[aicu_irq_ind]);
    m_icu->irq_in[aicu_irq_ind](m_aicu_irq[aicu_irq_ind]);
    aicu_irq_ind++;

    fprintf(stderr, "FrameBuffer on AICU_in%d\n", aicu_irq_ind);
    m_fb->irq (m_aicu_irq[aicu_irq_ind]);
    m_icu->irq_in[aicu_irq_ind](m_aicu_irq[aicu_irq_ind]);
    aicu_irq_ind++;

#if 0
    //interconnect
    onoc = new interconnect ("interconnect", is->no_cpus+2, nslaves);
	m_subsys->onoc   = onoc;

    for(i = 0; i < nslaves; i++)
        onoc->connect_slave_64 (i, slaves[i]);
#endif


    //masters
    m_qemu = new qemu_wrapper("QEMU", 0, is->no_cpus, m_int_cpu_mask, is->no_cpus, 
                              is->cpu_family, is->cpu_model, is->ramsize,
                              get_subsystem());
    m_qemu->add_map (0xC0000000, 0x10000000); // (base address, size)
    m_qemu->set_base_address (QEMU_ADDR_BASE);

    push_qemu_wrapper(m_qemu, is->no_cpus);

    for (i = 0; i < is->no_cpus; i++)
        m_qemu->interrupt_ports[i] (m_wires_irq_qemu[i]);


    for(i = 0; i < is->no_cpus; i++)
        push_master(m_qemu->get_cpu(i)->get_master());
    push_master(m_bl->get_master());
    push_master(m_fb->get_master());

    connect_devices(&addr_map);

    if(is->gdb_port > 0){
        m_qemu->m_qemu_import.gdb_srv_start_and_wait(m_qemu->m_qemu_instance,
                                                     is->gdb_port);
    }

#ifdef TRACE_EVENT_ENABLED 
    m_qemu->get_cpu(0)->systemc_qemu_write_memory(QEMU_ADDR_BASE + SET_SYSTEMC_INT_ENABLE,
                                                  0xFFFF /* 16 processors */, 4, 0, 0);
#else
    m_qemu->get_cpu(0)->systemc_qemu_write_memory(QEMU_ADDR_BASE + SET_SYSTEMC_INT_ENABLE,
                                                  0xFFFF /* 16 processors */, 4, 0);
#endif
}

void
bunny::end_of_elaboration(void)
{
	/* Loading the software */
    uint32_t mast_id = m_qemu->get_cpu(0)->get_node_id();
    app_loader loader(this);

    loader.load_elf_image(m_image_fname, mast_id, APPLOADER_DEFAULT);
}


bunny::~bunny()
{
    int i = 0;
    delete m_ram;
    delete m_bl;
    delete m_fb;
    delete m_tty0;
    delete m_ctrl0;
    delete m_mb;
    delete m_icu;

    for(i = 0; i < cpu_nb; i++){
        delete m_timers[i];
    }
    delete [] m_timers;

    delete m_qemu;
    delete [] m_aicu_irq;
    delete [] m_wires_irq_qemu;
    delete [] m_int_cpu_mask;
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
