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

#ifndef _QEMU_CPU_WRAPPER_
#define _QEMU_CPU_WRAPPER_

#include <cfg.h>
#include <qemu_wrapper_request.h>
#include <qemu_wrapper_access_interface.h>
#include <time.h>


class qemu_cpu_wrapper;
typedef qemu_cpu_wrapper qemu_cpu_wrapper_t;

#include <qemu_imported.h>

#include <master_device.h>

#ifdef TRACE_EVENT_ENABLED
#include <qemu_trace_port.h>
#endif

using namespace noc;

class cpu_logs;

class qemu_trace_port;

class qemu_cpu_wrapper : public master_device
{
public:
    SC_HAS_PROCESS (qemu_cpu_wrapper);
#ifdef TRACE_EVENT_ENABLED
    qemu_cpu_wrapper (sc_module_name name, qemu_instance *qemu_instance, unsigned int node_id,
                      int cpuindex, cpu_logs *logs, qemu_import_t *qi, cache_model_t *cache);
#else
    qemu_cpu_wrapper (sc_module_name name, qemu_instance *qemu_instance, unsigned int node_id,
                      int cpuindex, cpu_logs *logs, qemu_import_t *qi);
#endif
    ~qemu_cpu_wrapper ();

public:
    void set_base_address (uint32_t base_address);
    void set_cpu_fv_level (int val);
    int get_cpu_fv_level ();
    unsigned long get_cpu_ncycles ();
    void set_unblocking_write (bool val);
    void set_cycles_divisor (double div);

    //qemu interface
#ifdef TRACE_EVENT_ENABLED
    uint32_t systemc_qemu_read_memory (uint32_t addr, uint8_t nbytes,
                                       int bIO, hwe_cont* hwe_src);
    void systemc_qemu_write_memory (uint32_t addr, uint32_t data,
                                   uint8_t nbytes, int bIO, hwe_cont* hwe_src);
#else
    uint32_t systemc_qemu_read_memory (uint32_t address,
                                            uint8_t nbytes, int bIO);
    void systemc_qemu_write_memory (uint32_t address, uint32_t data,
                                    uint8_t nbytes, int bIO);
#endif

    void consume_instruction_cycles_with_sync (unsigned long ncycles);

    void update_events_with_sync(void);

    void add_time_at_fv (unsigned long ns);
    uint64 get_no_cycles ();
    void wait_wb_empty ();
    void wakeup ();
    void sync ();


    #ifdef ENERGY_TRACE_ENABLED
    //etrace
    void set_etrace_periph_id (unsigned long id);
    #endif

    #ifdef TRACE_EVENT_ENABLED
    void trace_port(void);
    qemu_trace_port_t *qemu_trace;
    #endif
    struct timespec start_sys_clock;


private:
    //threads
    void cpu_thread ();
#ifdef TRACE_EVENT_ENABLED
    virtual void rcv_rsp(uint8_t tid, uint8_t *data, bool bErr, bool bWrite, hwe_cont* hwe_rsp);
#else
    virtual void rcv_rsp(uint8_t tid, uint8_t *data, bool bErr, bool bWrite);
#endif


private:
    //local functions
#ifdef TRACE_EVENT_ENABLED
    uint32_t read  (uint32_t addr, uint8_t nbytes, int bIO, hwe_cont* hwe_src);
    void     write (uint32_t address, uint32_t data,
                    uint8_t nbytes, int bIO, hwe_cont* hwe_cont_ref);
#else
    uint32_t read  (uint32_t addr, uint8_t nbytes, int bIO);
    void write (uint32_t address, uint32_t data,
                uint8_t nbytes, int bIO);
#endif
    struct timespec timespec_diff(struct timespec start, struct timespec end);

public:
    //ports
    sc_port<qemu_wrapper_access_interface>  m_port_access;

private:
    //signals & events
    sc_event                                m_ev_wakeup;
    sc_event                                m_ev_sync;
    int                                     m_fv_level;

    //other attributes
    qemu_wrapper_requests                   *m_rqs;
    qemu_cpu_state_t                        *m_cpuenv;
    qemu_instance                           *m_qemu_instance;
    uint32_t                                m_base_address;
    uint32_t                                m_end_address;
    bool                                    m_unblocking_write;
    double                                  m_cycles_divisor;

    qemu_import_t                           *m_qemu_import;

    //tmp regs
    uint64                                  m_last_read_sctime;
    uint32_t                                m_last_no_cycles_high;

    //counters
    double                                  m_no_total_cycles;

    //log
    cpu_logs                                *m_logs;

    #ifdef ENERGY_TRACE_ENABLED
    //etrace
    unsigned long                           m_etrace_periph_id;
    #endif

public:
    int                                     m_cpuindex;
    unsigned long                           m_crt_cpu_thread;
    unsigned long                           m_swi;
 
};

//typedef qemu_cpu_wrapper< stbus_bca_request<64>, stbus_bca_response<64> > qemu_cpu_wrapper_t;

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
