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

#ifndef _QEMU_WRAPPER_
#define _QEMU_WRAPPER_

/*
 * TODO: Remove relative path (in Qemu)
 */
#include <cfg.h>
#include <qemu_wrapper_access_interface.h>
#include <qemu_imported.h>
#include <qemu_cpu_wrapper.h>
#include <../../qemu/sc_qemu/rabbits/systemc_imports.h>

using namespace noc;

#define DCACHE_LINES        1024
#define DCACHE_ASSOC_BITS   0
#define DCACHE_LINE_BITS    5
#define DCACHE_LINE_WORDS   (1 << (DCACHE_LINE_BITS - 2))
#define DCACHE_LINE_BYTES   (1 << DCACHE_LINE_BITS)
#define DCACHE_LINE_MASK    ((1 << DCACHE_LINE_BITS) - 1)

#define ICACHE_LINES        1024
#define ICACHE_ASSOC_BITS   0
#define ICACHE_LINE_BITS    5
#define ICACHE_LINE_WORDS   (1 << (ICACHE_LINE_BITS - 2))
#define ICACHE_LINE_BYTES   (1 << ICACHE_LINE_BITS)
#define ICACHE_LINE_MASK    ((1 << ICACHE_LINE_BITS) - 1)

class cpu_logs;

class qemu_wrapper : public sc_module, public qemu_wrapper_access_interface
{
public:
    SC_HAS_PROCESS (qemu_wrapper);
    qemu_wrapper (sc_module_name name, unsigned int node, int ninterrupts, int *int_cpu_mask,
                  int nocpus, const char *cpufamily, const char *cpumodel, int ramsize,
                  generic_subsystem *subsys);
    ~qemu_wrapper ();

public:
    void add_map (uint32_t base_address, uint32_t size);
    void set_base_address (uint32_t base_address);
    void set_unblocking_write (bool val);
    void set_cycles_divisor (double div);

    //inline qemu_cpu_wrapper <stbus_bca_request<64>, stbus_bca_response<64> > * get_cpu (int i) {return m_cpus[i];}
    inline qemu_cpu_wrapper_t * get_cpu (int i) {return m_cpus[i];}

#ifdef TRACE_EVENT_ENABLED
    void invalidate_address (uint32_t addr, int idx_src, hwe_cont* hwe);
#else
    void invalidate_address (uint32_t addr, int idx_src);
#endif


    //qemu_wrapper_access_interface
    virtual int get_no_cpus  ();
    virtual unsigned long get_cpu_fv_level (int cpu);
    virtual void set_cpu_fv_level (int cpu, unsigned long val);
    virtual void generate_swi (unsigned long cpu_mask, unsigned long swi);
    virtual void swi_ack (int cpu, unsigned long swi_mask);
    virtual unsigned long get_cpu_ncycles (int cpu);
    virtual unsigned long get_int_status ();
    virtual unsigned long get_int_enable ();
    virtual void set_int_enable (unsigned long val);
    virtual uint64 get_no_cycles_cpu (int cpu);
#ifdef TRACE_EVENT_ENABLED
    virtual bool get_trace_enable(void);
    virtual void set_trace_enable(bool enable);
#endif

private:
    //threads
    void interrupts_thread ();
    void timeout_thread ();

    // interrupts_thread variables
    bool              *m_bup;
    bool              *m_bdown;
    sc_event_or_list   m_event_list;

public:
    //ports
    sc_in<bool>                         *interrupt_ports;

    qemu_instance                      *m_qemu_instance;
    qemu_import_t                       m_qemu_import;

    static qemu_wrapper                 *s_wrappers[256];    /* TODO: Remove Static */
    static int                           s_nwrappers;        /* TODO: Remove Static */

    int                                 m_ncpu;
    qemu_cpu_wrapper_t                  **m_cpus;

private:
    // attr
    unsigned long                       *m_cpu_interrupts_raw_status;
    unsigned long                       *m_cpu_interrupts_status;
    unsigned long                       m_interrupts_raw_status;
    unsigned long                       m_interrupts_enable;
    int                                 *m_irq_cpu_mask;

    void                                *m_lib_handle;

    int                                 m_ninterrupts;
    bool                                m_qemuLoaded;

    cpu_logs                            *m_logs;
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
