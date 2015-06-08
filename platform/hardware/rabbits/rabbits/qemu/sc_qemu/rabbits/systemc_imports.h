#ifndef _SYSTEMC_IMPORTS_H_
#define _SYSTEMC_IMPORTS_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Basic types for loopback
 */

#ifndef __cplusplus
typedef struct generic_subsystem generic_subsystem_t;
typedef struct qemu_cpu_wrapper qemu_cpu_wrapper_t;
#else
#include <generic_subsystem.h>
#include <qemu_cpu_wrapper.h>
#endif

#if defined TRACE_EVENT_ENABLED || defined RABBITS_TRACE_EVENT
#include <events/hwe_events.h>
#endif


#if 0
typedef void            (*systemc_qemu_wakeup_fc_t) (void *sc_obj);
#endif
typedef void            (*systemc_qemu_consume_instruction_cycles_fc_t) (qemu_cpu_wrapper_t *sc_obj, int ninst);
typedef void            (*systemc_qemu_consume_ns_fc_t) (unsigned long ns);


#if defined TRACE_EVENT_ENABLED || defined RABBITS_TRACE_EVENT
typedef uint32_t        (*systemc_qemu_read_memory_fc_t) (qemu_cpu_wrapper_t *sc_obj, uint32_t address, uint8_t nbytes, int bIO, hwe_cont* hwe_src);
typedef void            (*systemc_qemu_write_memory_fc_t) (qemu_cpu_wrapper_t *sc_obj, uint32_t address, uint32_t data, unsigned char nbytes, int bIO, hwe_cont* hwe_src);
typedef void            (*systemc_trace_event_fc_t) ( void *sc_obj ); // CUNHA
#else
typedef uint32_t        (*systemc_qemu_read_memory_fc_t) (qemu_cpu_wrapper_t *sc_obj, uint32_t address, uint8_t nbytes, int bIO);
typedef void            (*systemc_qemu_write_memory_fc_t) (qemu_cpu_wrapper_t *sc_obj, uint32_t address, uint32_t data, unsigned char nbytes, int bIO);
#endif
typedef unsigned long long  (*systemc_qemu_get_time_fc_t) (void);
typedef uintptr_t       (*systemc_get_mem_addr_fc_t) (qemu_cpu_wrapper_t *sc_obj, generic_subsystem_t *sub, uint32_t addr);
typedef unsigned long   (*systemc_qemu_get_crt_thread_fc_t) (qemu_cpu_wrapper_t *qw);
typedef void            (*memory_mark_exclusive_fc_t)  (generic_subsystem_t *sub, int cpu, uint32_t addr);
typedef int             (*memory_test_exclusive_fc_t)  (generic_subsystem_t *sub, int cpu, uint32_t addr);
typedef void            (*memory_clear_exclusive_fc_t) (generic_subsystem_t *sub, int cpu, uint32_t addr);
typedef void            (*wait_wb_empty_fc_t) (qemu_cpu_wrapper_t *sc_obj);
typedef void            (*systemc_stop_t) (void);

/*
 *
 */
struct systemc_import_t
{
    qemu_cpu_wrapper_t                             *qemu_cpu_wrap;
	generic_subsystem_t                            *subsystem; 
    systemc_qemu_consume_instruction_cycles_fc_t    systemc_qemu_consume_instruction_cycles;
    systemc_qemu_consume_ns_fc_t                    systemc_qemu_consume_ns;
    systemc_qemu_read_memory_fc_t                   systemc_qemu_read_memory;
    systemc_qemu_write_memory_fc_t                  systemc_qemu_write_memory;
    systemc_qemu_get_time_fc_t                      systemc_qemu_get_time;
    systemc_get_mem_addr_fc_t                       systemc_get_mem_addr;
    systemc_qemu_get_crt_thread_fc_t                systemc_qemu_get_crt_thread;
#if defined TRACE_EVENT_ENABLED || defined RABBITS_TRACE_EVENT
    systemc_trace_event_fc_t                        systemc_trace_event; // CUNHA
#endif
    memory_mark_exclusive_fc_t                      memory_mark_exclusive;
    memory_test_exclusive_fc_t                      memory_test_exclusive;
    memory_clear_exclusive_fc_t                     memory_clear_exclusive;
    wait_wb_empty_fc_t                              wait_wb_empty;
#if 0
    systemc_qemu_wakeup_fc_t                        systemc_qemu_wakeup;
#endif
	systemc_stop_t                                  systemc_stop;
};

#ifdef __cplusplus
}
#endif

#endif
