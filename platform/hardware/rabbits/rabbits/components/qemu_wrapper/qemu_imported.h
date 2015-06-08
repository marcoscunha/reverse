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

#ifndef _QEMU_IMPORTED_FUNCTIONS_
#define _QEMU_IMPORTED_FUNCTIONS_

#include <inttypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct qemu_import_t qemu_import_t;
    typedef struct qemu_instance qemu_instance;
    typedef struct systemc_import_t systemc_import_t;

    struct qemu_counters_t;
    struct qemu_perf_t;
#if defined RABBITS_TRACE_EVENT ||  defined TRACE_EVENT_ENABLED
    struct qemu_trace_t;
#endif

#if defined RABBITS_TRACE_EVENT || defined TRACE_EVENT_ENABLED
    typedef struct qemu_tr_buf qemu_tr_buf_t;
#endif

    typedef struct cache  cache_t;
    typedef struct cache_model cache_model_t;

/*
 * TODO: Place it somewhere else
 */
#ifdef __cplusplus
    typedef struct cpu_state qemu_cpu_state_t;
#else
    typedef CPUState qemu_cpu_state_t;
#endif
    
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <qemu_cpu_wrapper.h>

extern "C"
{
#endif

#if defined RABBITS_TRACE_EVENT || defined TRACE_EVENT_ENABLED
#include "../trace_port/trace_port.h"

/**
 * @struct Ring buffer used to manage the events between SystemC and QEMU
 *
 */
struct qemu_tr_buf
{
    uint32_t    size;    /** Buffer size in number of elements                */
    uint32_t    head;    /** Index of fist ring buffer element                */
    uint32_t    tail;    /** Index of last ring buffer element                */
    void**      data;    /** Pointer to data structure pointer                */
    uint8_t     last_op; /** Last operation 0 to read and 1 to write          */
    bool        timestamp_updated; /**< Events with timestamp data            */
};

#endif /* RABBITS_TRACE_EVENT */

/**
 * @struct Struct device definition for cache used by QEMU and LIBTRACE
 *
 */
struct cache 
{
    uint32_t lines;     /**<  Number of cache lines                           */
    uint32_t line_bits; /**<  Number of line bits                             */
    uint32_t assoc_bits;/**<  */
};

/**
 * @struct This struct definition must contains the data and instruction cache
 * model configurations.
 *
 */
struct cache_model
{
    cache_t data;       /**< Data Cache Model                                 */
    cache_t instr;      /**< Instruction Cache Model                          */
};

#if defined RABBITS_TRACE_EVENT || defined TRACE_EVENT_ENABLED
    typedef qemu_instance  *(*qemu_init_fc_t) (int id, int ncpu,
                                                const char *cpu_model, int _ramsize, 
                                               cache_model_t *cache, trace_port_t** trace_port, struct qemu_import_t *qi, systemc_import_t *systemc_fcs);
#else
    typedef qemu_instance  *(*qemu_init_fc_t) (int id, int ncpu,
                                                const char *cpu_model, int _ramsize, 
                                                struct qemu_import_t *qi, systemc_import_t *systemc_fcs);
#endif
    typedef void            (*qemu_add_map_fc_t) (qemu_instance *instance,
                                                  uint32_t base_address, uint32_t size, 
                                                  int type);
    typedef void            (*qemu_release_fc_t)(qemu_instance *instance);
    typedef qemu_cpu_state_t *(*qemu_get_set_cpu_obj_fc_t)(qemu_instance *instance,
                                                           unsigned long index, qemu_cpu_wrapper_t *sc_obj);
    typedef void            (*qemu_cpu_start_fc_t) (qemu_cpu_state_t *cpuenv, unsigned long index);

    typedef long            (*qemu_cpu_loop_fc_t) (qemu_cpu_state_t *cpuenv);
    typedef void            (*qemu_irq_update_fc_t)(qemu_instance *instance, int cpu_mask,
                                                     int level);
    typedef struct qemu_counters_t *(*qemu_get_counters_fc_t)(qemu_instance *instance);
#if defined TRACE_EVENT_ENABLED  || defined RABBITS_TRACE_EVENT
    typedef void            (*qemu_invalidate_address_fc_t)(qemu_instance* instance,
                                                     uint32_t addr, int src_idx,
                                                     tr_event_grp_t type, hwe_cont* hwe,
                                                     uint64_t timestamp);
#else
    typedef void            (*qemu_invalidate_address_fc_t)(qemu_instance* instance, 
                                                             uint32_t addr, int src_idx);
#endif
    typedef int             (*gdb_srv_start_and_wait_fc_t)(qemu_instance *instance, int port);
#if defined TRACE_EVENT_ENABLED || defined RABBITS_TRACE_EVENT
    typedef qemu_tr_buf_t *(*qemu_get_tr_buf_fc_t) (qemu_instance *instance, int cpu);
    typedef struct qemu_trace_t *(*qemu_get_set_trace_fc_t)(qemu_instance *instance, uint8_t cmd);

#endif

     typedef struct qemu_perf_t *(*qemu_get_perf_fc_t)(qemu_instance *instance);
   
    //imported by QEMU
    struct qemu_counters_t
    {
        uint64_t            no_instructions;
        uint64_t            no_cycles;
        uint64_t            no_mem_write;
        uint64_t            no_mem_read;
        uint64_t            no_dcache_miss;
        uint64_t            no_icache_miss;
        uint64_t            no_io_write;
        uint64_t            no_io_read;

    };

//   #ifdef RABBITS_PERF
   struct qemu_perf_t{
//        struct timespec     translation;
//        struct timespec     execution;
        struct timespec     tlm;
   };
// #endif
#if defined TRACE_EVENT_ENABLED || defined RABBITS_TRACE_EVENT
   struct qemu_trace_t{
       bool enable;
   };
#endif

    struct qemu_import_t
    {
        qemu_init_fc_t                  qemu_init;
        qemu_add_map_fc_t               qemu_add_map;
        qemu_release_fc_t               qemu_release;
        qemu_get_set_cpu_obj_fc_t       qemu_get_set_cpu_obj;
        qemu_cpu_start_fc_t             qemu_cpu_start;
        qemu_cpu_loop_fc_t              qemu_cpu_loop;
        qemu_irq_update_fc_t            qemu_irq_update;
        qemu_get_counters_fc_t          qemu_get_counters;
        qemu_invalidate_address_fc_t    qemu_invalidate_address;
        gdb_srv_start_and_wait_fc_t     gdb_srv_start_and_wait;
#if defined TRACE_EVENT_ENABLED || defined RABBITS_TRACE_EVENT
        qemu_get_tr_buf_fc_t            qemu_get_tr_buf;
        qemu_get_set_trace_fc_t         qemu_get_set_trace;
#endif
//#ifdef RABBITS_PERF
       qemu_get_perf_fc_t               qemu_get_perf;
//#endif
    };

    void rabbits_exit(void);
#ifdef __cplusplus
}
#endif

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
