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

#include <cfg.h>
#include <qemu_wrapper.h>
#include <qemu_cpu_wrapper.h>
#include <qemu_imported.h>
#include <qemu_wrapper_request.h>
#include <qemu_wrapper_cts.h>
#include <cpu_logs.h>
#include <dlfcn.h>
#include <systemc>

#ifdef ENERGY_TRACE_ENABLED
#include <etrace_if.h>
#endif

#ifdef TRACE_EVENT_ENABLED
#include <qemu_trace_port.h>
#endif

#include <../../qemu/sc_qemu/rabbits/systemc_imports.h>

//#define _DEBUG_WRAPPER_QEMU_

#ifdef _DEBUG_WRAPPER_QEMU_
#define DCOUT cout
#else
#define DCOUT if (0) cout
#endif

#if defined (ENERGY_TRACE_ENABLED) && !defined (ETRACE_NB_CPU_IN_GROUP)
#define ETRACE_NB_CPU_IN_GROUP 4
#endif

#define CPU_TIMEOUT         20000000

qemu_wrapper                *qemu_wrapper::s_wrappers[256];
int                         qemu_wrapper::s_nwrappers = 0;

extern "C"
{

    void qemu_wrapper_SLS_banner (void) __attribute__ ((constructor));

    extern void systemc_qemu_wakeup (qemu_cpu_wrapper_t *_this);
    extern void systemc_qemu_consume_instruction_cycles (
        qemu_cpu_wrapper_t *_this, int ninstr);
    extern void systemc_qemu_consume_ns (unsigned long ns);

#ifdef TRACE_EVENT_ENABLED
    extern uint32_t systemc_qemu_read_memory (qemu_cpu_wrapper_t *_this, 
                                              uint32_t address, uint8_t nbytes, int bIO, hwe_cont* hwe_src);
    extern void systemc_qemu_write_memory (qemu_cpu_wrapper_t *_this, 
                                           uint32_t address, uint32_t data, uint8_t nbytes, int bIO, hwe_cont* hwe_src);
#else
    extern uint32_t systemc_qemu_read_memory (qemu_cpu_wrapper_t *_this, 
        uint32_t address, uint8_t nbytes, int bIO);
    extern void systemc_qemu_write_memory (qemu_cpu_wrapper_t *_this, 
        uint32_t address, uint32_t data, uint8_t nbytes, int bIO);
#endif

    extern unsigned long long systemc_qemu_get_time ();
    extern unsigned long systemc_qemu_get_crt_thread (qemu_cpu_wrapper_t *_this);

#ifdef TRACE_EVENT_ENABLED
    extern void systemc_trace_event(void);
#endif

    extern void wait_wb_empty (qemu_cpu_wrapper_t *_this);

    extern uintptr_t systemc_get_mem_addr  (qemu_cpu_wrapper_t *qw, generic_subsystem_t *sub, uint32_t addr);
    extern void      memory_mark_exclusive (generic_subsystem_t *sub, int cpu, uint32_t addr);
    extern int       memory_test_exclusive (generic_subsystem_t *sub, int cpu, uint32_t addr);
    extern void      memory_clear_exclusive(generic_subsystem_t *sub, int cpu, uint32_t addr);

    void
    rabbits_exit(void){
        printf("Rabbits Exiting\n");
        sc_stop();
    }
}

/**
 * @brief
 *
 * @param name
 * @param node
 * @param ninterrupts
 * @param int_cpu_mask
 * @param nocpus
 * @param cpufamily
 * @param cpumodel
 * @param ramsize
 */
qemu_wrapper::qemu_wrapper (sc_module_name name, unsigned int node, 
                            int ninterrupts, int *int_cpu_mask, int nocpus,
                            const char *cpufamily, const char *cpumodel, int ramsize,
                            generic_subsystem_t *subsys)
: sc_module (name)
{

#ifdef TRACE_EVENT_ENABLED
    cache_model_t cache;
    trace_port_t** trace_port;
#endif

    m_ncpu = nocpus;
    m_qemuLoaded = false;
    m_ninterrupts = ninterrupts;
    interrupt_ports = NULL;
    m_irq_cpu_mask = 0;
    s_wrappers[s_nwrappers++] = this;

    m_interrupts_raw_status = 0;
    m_interrupts_enable = 0;
    if (m_ninterrupts)
    {
        interrupt_ports = new sc_in<bool> [m_ninterrupts];
        m_irq_cpu_mask = new int[m_ninterrupts];
        for (int i = 0; i < m_ninterrupts; i++)
            m_irq_cpu_mask[i] = int_cpu_mask[i];
    }

    char    buf[200];
    sprintf (buf, "libqemu-system-%s.so", cpufamily);

    m_lib_handle = dlopen (buf, RTLD_NOW);

    if (!m_lib_handle)
    {
        printf ("Cannot load library %s in %s\n", buf, __FUNCTION__);
        exit (1);
    }
    dlerror();
    sprintf (buf, "%s_qemu_init", cpufamily);

    m_qemu_import.qemu_init = (qemu_init_fc_t) dlsym (m_lib_handle, buf);

    if (!m_qemu_import.qemu_init)
    {
        printf ("Cannot load %s symbol from library libqemu-system-%s.so in %s\n",
            buf, cpufamily, __FUNCTION__);
        exit (1);
    }

    systemc_import_t        sc_exp_fcs;
#if 0
    sc_exp_fcs.systemc_qemu_wakeup = (systemc_qemu_wakeup_fc_t) systemc_qemu_wakeup;
#endif
    sc_exp_fcs.systemc_qemu_consume_instruction_cycles = systemc_qemu_consume_instruction_cycles;
    sc_exp_fcs.systemc_qemu_consume_ns = systemc_qemu_consume_ns;
    sc_exp_fcs.systemc_qemu_read_memory = systemc_qemu_read_memory;
    sc_exp_fcs.systemc_qemu_write_memory = systemc_qemu_write_memory;
    sc_exp_fcs.systemc_qemu_get_time = systemc_qemu_get_time;
    sc_exp_fcs.systemc_get_mem_addr = systemc_get_mem_addr;
    sc_exp_fcs.systemc_qemu_get_crt_thread = systemc_qemu_get_crt_thread;
#ifdef TRACE_EVENT_ENABLED
    sc_exp_fcs.systemc_trace_event = (systemc_trace_event_fc_t) systemc_trace_event; // CUNHA: TRACE PORT
#endif

    sc_exp_fcs.memory_mark_exclusive = memory_mark_exclusive;
    sc_exp_fcs.memory_test_exclusive = memory_test_exclusive;
    sc_exp_fcs.memory_clear_exclusive = memory_clear_exclusive;
    sc_exp_fcs.wait_wb_empty = wait_wb_empty;
    sc_exp_fcs.systemc_stop = rabbits_exit;

#ifdef TRACE_EVENT_ENABLED
    // Initialization Cache Parameters
    cache.instr.lines     = ICACHE_LINES;
    cache.instr.line_bits = ICACHE_LINE_BITS;
    cache.instr.assoc_bits= ICACHE_ASSOC_BITS;
    cache.data.lines      = DCACHE_LINES;
    cache.data.line_bits  = DCACHE_LINE_BITS;
    cache.data.assoc_bits = DCACHE_ASSOC_BITS;

    trace_port = (trace_port_t**)malloc(sizeof(trace_port_t)*m_ncpu);

    for(int i = 0; i < m_ncpu; i++){
        trace_port_t *port;
        trace_port[i] = (trace_port_t*)calloc(1,sizeof(trace_port_t));

        port = trace_port[i];
        // TRACE Library
        hwe_device_t type;
        hwe_devices_u dev;
        char str[20];

        // CPU Description
        sprintf(str, "PROCESSOR_%d", i);
        type = HWE_PROCESSOR;
        dev.processor.cpuid = i;
        port->cpu = hwe_port_open (str, type, &dev);

        // Data Cache description
        sprintf(str, "DCACHE_CPU_%d", i);
        type = HWE_CACHE;
        dev.cache.numset  = 1;
        dev.cache.numline = cache.data.lines -1 ;
        dev.cache.numbyte = (1 << cache.data.line_bits)- 1;
        port->dcache = hwe_port_open (str, type, &dev);

        // Instruction Cache description
        type = HWE_CACHE;
        sprintf(str, "ICACHE_CPU_%d", i);
        dev.cache.numset  = 1;
        dev.cache.numline = cache.instr.lines - 1;
        dev.cache.numbyte = (1 << cache.instr.line_bits) -1;
        port->icache = hwe_port_open (str, type, &dev);
    }
#endif

    sc_exp_fcs.subsystem = subsys;

#ifdef TRACE_EVENT_ENABLED
    m_qemu_instance = m_qemu_import.qemu_init (node, m_ncpu,
            cpumodel, ramsize, &cache, trace_port, &m_qemu_import, &sc_exp_fcs);
#else
    m_qemu_instance = m_qemu_import.qemu_init (node, m_ncpu,
                                               cpumodel, ramsize, &m_qemu_import, &sc_exp_fcs);
#endif

    printf ("QEMU %d called %s has %d %s %s ([node_id, cpu_id] = ",
        s_nwrappers - 1, (const char *)name, m_ncpu, cpufamily, m_ncpu == 1 ? "cpu" : "cpus");
    for (int i = 0; i < m_ncpu; i++)
    {
        if (i)
            printf (", ");
        printf ("[%d, %d]", node + i, i);
    }
    printf (")\n");
    m_qemuLoaded = true;

    m_cpu_interrupts_status     = new unsigned long[m_ncpu];
    m_cpu_interrupts_raw_status = new unsigned long[m_ncpu];
    for (int i = 0; i < m_ncpu; i++)
    {
        m_cpu_interrupts_status[i]     = 0;
        m_cpu_interrupts_raw_status[i] = 0;
    }

    m_logs = new cpu_logs (m_ncpu, cpufamily, cpumodel);

    #ifdef ENERGY_TRACE_ENABLED
    int             etrace_group_id;
    unsigned long   periph_id;
    char            etrace_group_name[50];
    #endif

    m_cpus = new qemu_cpu_wrapper_t * [m_ncpu];
    for (int i = 0; i < m_ncpu; i++)
    {
        char s[50];
        sprintf (s, "qemu-cpu-%d", i);
#ifdef TRACE_EVENT_ENABLED
        m_cpus[i] = new qemu_cpu_wrapper_t (s, m_qemu_instance, node + i, i,
                                            m_logs, &m_qemu_import, &cache);
#else
        m_cpus[i] = new qemu_cpu_wrapper_t (s, m_qemu_instance, node + i, i,
                                            m_logs, &m_qemu_import);
#endif
        m_cpus[i]->m_port_access (*this);

        #ifdef ENERGY_TRACE_ENABLED
        if ((i % ETRACE_NB_CPU_IN_GROUP) == 0)
        {
            etrace_group_id = -1;
            int end_cpu = i + ETRACE_NB_CPU_IN_GROUP - 1;
            if (end_cpu > m_ncpu - 1)
                end_cpu = m_ncpu - 1;
            if (i == end_cpu)
                sprintf (etrace_group_name, "CPU %d", i);
            else
                sprintf (etrace_group_name, "CPU %d-%d", i, end_cpu);
        }
        sprintf (buf, "CPU %d", i);
        periph_id = etrace.add_periph (buf,
            get_cpu_etrace_class (cpufamily, cpumodel),
            etrace_group_id, etrace_group_name);
        m_cpus[i]->set_etrace_periph_id (periph_id);
        #endif
    }

    /*
     * interrupt_threads vars
     */
    m_bup = new bool[m_ncpu];
    m_bdown = new bool[m_ncpu];

    SC_THREAD (interrupts_thread);
    SC_THREAD (timeout_thread);
}

qemu_wrapper::~qemu_wrapper ()
{
    int                 i;

    if (m_qemuLoaded)
        m_qemu_import.qemu_release (m_qemu_instance);

    delete m_logs;
    if (m_cpus)
    {
        for (i = 0; i < m_ncpu; i++)
            delete m_cpus [i];

        delete [] m_cpus;
    }

    if (interrupt_ports)
        delete [] interrupt_ports;

    if (m_irq_cpu_mask)
        delete [] m_irq_cpu_mask;

    delete [] m_cpu_interrupts_raw_status;
    delete [] m_cpu_interrupts_status;
    
    delete [] m_bup;
    delete [] m_bdown;

    if (m_lib_handle)
        dlclose (m_lib_handle);
}

void qemu_wrapper::set_unblocking_write (bool val)
{
    int                 i;
    for (i = 0; i < m_ncpu; i++)
        m_cpus [i]->set_unblocking_write (val);
}

void qemu_wrapper::set_cycles_divisor (double div)
{
    int                 i;
    for (i = 0; i < m_ncpu; i++)
        m_cpus [i]->set_cycles_divisor (div);
}

void qemu_wrapper::add_map (uint32_t base_address, uint32_t size)
{
    m_qemu_import.qemu_add_map (m_qemu_instance, base_address, size, 0);
}

void qemu_wrapper::set_base_address (uint32_t base_address)
{
    m_qemu_import.qemu_add_map (m_qemu_instance, base_address, SIZE_QEMU_WRAPPER_MEMORY, 1);

    for (int i = 0; i < m_ncpu; i++)
        m_cpus[i]->set_base_address (base_address);
}

/**
 * timeout_thread used to log information
 * - Timeout thread that waits for CPU_TIMEOUT to proceds the log and
 *   syncronyzation
 * -
 */
void qemu_wrapper::timeout_thread ()
{
    #ifdef TIME_AT_FV_LOG_GRF
    while (1)
    {
        wait (CPU_TIMEOUT, SC_NS);

        for (int i = 0; i < m_ncpu; i++)
            m_cpus[i]->sync ();
        wait (0, SC_NS);

        m_logs->update_fv_grf ();
    }
    #endif
}

void qemu_wrapper::interrupts_thread ()
{
    if (!m_ninterrupts)
        return;

    int               i, cpu;
    unsigned long     val;

    for (i = 0; i < m_ninterrupts; i++)
        m_event_list |= interrupt_ports[i].default_event ();

    while (1)
    {
        wait (m_event_list);

        for (cpu = 0; cpu < m_ncpu; cpu++)
        {
            m_bup[cpu] = false;
            m_bdown[cpu] = false;
        }

        val = 1;
        for (i = 0; i < m_ninterrupts; i++)
        {
            if (interrupt_ports[i].posedge ())
            {
                m_interrupts_raw_status |= val;
                for (cpu = 0; cpu < m_ncpu; cpu++)
                {
                    if (m_irq_cpu_mask[i] & (1 << cpu))
                    {
                        m_cpu_interrupts_raw_status[cpu] |= val;
                        if (val & m_interrupts_enable)
                        {
                            if (!m_cpu_interrupts_status[cpu]){
                                m_bup[cpu] = true;
                            }
                            m_cpu_interrupts_status[cpu] |= val;
                        }
                    }
                }
            }
            else
                if (interrupt_ports[i].negedge ())
                {
                    m_interrupts_raw_status &= ~val;
                    for (cpu = 0; cpu < m_ncpu; cpu++)
                    {
                        if (m_irq_cpu_mask[i] & (1 << cpu))
                        {
                            m_cpu_interrupts_raw_status[cpu] &= ~val;
                            if (val & m_interrupts_enable)
                            {
                                m_cpu_interrupts_status[cpu] &= ~val;
                                if (!m_cpu_interrupts_status[cpu])
                                    m_bdown[cpu] = true;
                            }
                        }
                    }
                }

            val <<= 1;
        }

        for (cpu = 0; cpu < m_ncpu; cpu++)
        {
            if (m_bup[cpu] && !m_cpus[cpu]->m_swi)
            {
                DCOUT << "******INT SENT***** to cpu " << cpu << endl;
                m_qemu_import.qemu_irq_update (m_qemu_instance, 1 << cpu, 1);
                m_cpus[cpu]->wakeup ();
            }
            else
                if (m_bdown[cpu] && !m_cpus[cpu]->m_swi)
                {
                    //wait (2.5, SC_NS);
                    m_qemu_import.qemu_irq_update (m_qemu_instance, 1 << cpu, 0);
                }
        }
    }
}

//qemu_wrapper_access_interface
int qemu_wrapper::get_no_cpus  ()
{
    return m_ncpu;
}

unsigned long qemu_wrapper::get_cpu_fv_level (int cpu)
{
    return m_cpus[cpu]->get_cpu_fv_level ();
}

void qemu_wrapper::set_cpu_fv_level (int cpu, unsigned long val)
{
    if (cpu == -1)
    {
        for (cpu = 0; cpu < m_ncpu; cpu++)
            m_cpus[cpu]->set_cpu_fv_level (val);
    }
    else
        m_cpus[cpu]->set_cpu_fv_level (val);
}

void qemu_wrapper::generate_swi (unsigned long cpu_mask, unsigned long swi)
{
    int                         cpu;
    for (cpu = 0; cpu < m_ncpu; cpu++)
    {
        if (cpu_mask & (1 << cpu))
        {
            m_cpus[cpu]->m_swi |= swi;
            if (!m_cpu_interrupts_status[cpu])
            {
                m_qemu_import.qemu_irq_update (m_qemu_instance, 1 << cpu, 1);
                m_cpus[cpu]->wakeup ();
            }
        }
    }
}

void qemu_wrapper::swi_ack (int cpu, unsigned long swi_mask)
{
    unsigned long   swi = m_cpus[cpu]->m_swi;
    if (swi == 0)
        return;
    swi &= ~swi_mask;
    m_cpus[cpu]->m_swi = swi;

    if (!m_cpu_interrupts_status[cpu] && !swi)
        m_qemu_import.qemu_irq_update (m_qemu_instance, 1 << cpu, 0);
}

unsigned long qemu_wrapper::get_cpu_ncycles (int cpu)
{
    return m_cpus[cpu]->get_cpu_ncycles ();
}

uint64 qemu_wrapper::get_no_cycles_cpu (int cpu)
{
    uint64                      ret = 0;
    if (cpu == -1)
    {
        for (int i = 0; i < s_nwrappers; i++)
            for (cpu = 0; cpu < s_wrappers[i]->m_ncpu; cpu++)
                ret += s_wrappers[i]->m_cpus[cpu]->get_no_cycles ();
    }
    else
        ret = m_cpus[cpu]->get_no_cycles ();

    return ret;
}

unsigned long qemu_wrapper::get_int_status ()
{
    return m_interrupts_raw_status & m_interrupts_enable;
}

unsigned long qemu_wrapper::get_int_enable ()
{
    return m_interrupts_enable;
}

void qemu_wrapper::set_int_enable (unsigned long val)
{
    int            cpu;
    for (cpu = 0; cpu < m_ncpu; cpu++)
    {
        if (!m_cpu_interrupts_status[cpu] && (m_cpu_interrupts_raw_status[cpu] & val)
            && !m_cpus[cpu]->m_swi)
            m_qemu_import.qemu_irq_update (m_qemu_instance, 1 << cpu, 1);
        else
            if (m_cpu_interrupts_status[cpu] && !(m_cpu_interrupts_raw_status[cpu] & val)
                && !m_cpus[cpu]->m_swi)
                m_qemu_import.qemu_irq_update (m_qemu_instance, 1 << cpu, 0);

        m_cpu_interrupts_status[cpu] = m_cpu_interrupts_raw_status[cpu] & val;
    }

    m_interrupts_enable = val;
}

#ifdef TRACE_EVENT_ENABLED
bool qemu_wrapper::get_trace_enable(void)
{
    m_qemu_import.qemu_get_set_trace (m_qemu_instance, 0);
    return true;
}

void qemu_wrapper::set_trace_enable (bool enable)
{
    // TODO: Syncronize all processors before start the trace generation
    m_qemu_import.qemu_get_set_trace (m_qemu_instance, 1);

}
#endif



#ifdef TRACE_EVENT_ENABLED
void
qemu_wrapper::invalidate_address (uint32_t addr, int idx_src, hwe_cont* hwe)
#else
void
qemu_wrapper::invalidate_address (uint32_t addr, int idx_src)
#endif
{
#ifdef TRACE_EVENT_ENABLED
   // FIXME: Used also to Instruction Cache despite the macro
   // TR_EVNT_DCACHE_INV ( corrected in function)
    m_qemu_import.qemu_invalidate_address (m_qemu_instance,
                addr, idx_src, TR_EVNT_DCACHE_INV, hwe, sc_time_stamp().value());
#else
    m_qemu_import.qemu_invalidate_address (m_qemu_instance,
                addr, idx_src);
#endif

}

extern "C"
{
    void
    qemu_wrapper_SLS_banner (void)
    {
        fprintf (stdout,
                "================================================================================\n"
                "|  This simulation uses the QEMU/SystemC Wrapper from the RABBITS' framework   |\n"
                "|                     Copyright (c) 2009 - 2010 Tima SLS                       |\n"
                "================================================================================\n");
    }
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
