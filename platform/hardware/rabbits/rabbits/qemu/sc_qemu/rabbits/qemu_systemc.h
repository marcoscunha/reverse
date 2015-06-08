#ifndef _QEMU_SYSTEMC_H_
#define _QEMU_SYSTEMC_H_

#include "qemu-common.h"
#include "cpu.h"
#include "rabbits/qemu_encap.h"
#include "../../../components/qemu_wrapper/qemu_imported.h"

#include <time.h>
//#define DEBUG_TB_ALLOC
#ifdef DEBUG_TB_ALLOC
    #define DTBALLOCPRINTF printf
#else
    #define DTBALLOCPRINTF if (0) printf
#endif

#ifdef RABBITS_PERF

static inline void t_diff(struct timespec *start, struct timespec *end, struct timespec *result)
{
    if ((end->tv_nsec-start->tv_nsec)<0) {
        result->tv_sec = end->tv_sec-start->tv_sec-1;
        result->tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec;
    } else {
        result->tv_sec = end->tv_sec-start->tv_sec;
        result->tv_nsec = end->tv_nsec-start->tv_nsec;
    }
}

static inline void t_add(struct timespec *t1, struct timespec *t2 )
{
    t1->tv_sec += t2->tv_sec;
    t1->tv_nsec += t2->tv_nsec;
    if (t1->tv_nsec >= 1000000000L) {        /* Carry? */        
        t1->tv_sec++ ;
        t1->tv_nsec -= 1000000000L ;
        if(t1->tv_nsec >= 1000000000L){
            printf("Overflow\n");
        }
    }
}


#define PRINT_TIMESPEC(time) printf("%lld.%.9ld\n", (long long)time.tv_sec, time.tv_nsec)
#define START_EXEC_PERF()  /*clock_gettime(CLOCK_MONOTONIC, &tmp_start)*/
#define STOP_EXEC_PERF()   /*clock_gettime(CLOCK_MONOTONIC, &tmp_end);\
                            t_diff(&tmp_start,&tmp_end, &tmp_diff);\
                            t_add(&crt_qemu_instance->perf.execution, &tmp_diff);*/
/*                            PRINT_TIMESPEC(crt_qemu_instance->perf.execution);*/
#define START_TRANS_PERF()  clock_gettime(CLOCK_REALTIME_COARSE, &trans_start)
#define STOP_TRANS_PERF()   clock_gettime(CLOCK_REALTIME_COARSE, &trans_end);\
                            t_diff(&trans_start,&trans_end, &trans_diff);\
                            t_add(&crt_qemu_instance->perf.translation, &trans_diff);
//                           PRINT_TIMESPEC(crt_qemu_instance->perf.translation);
#define START_TLM_PERF()   clock_gettime(CLOCK_REALTIME_COARSE, &tlm_start)
#define STOP_TLM_PERF()    clock_gettime(CLOCK_REALTIME_COARSE, &tlm_end);\
                            t_diff(&tlm_start,&tlm_end, &tlm_diff);\
                            t_add(&crt_qemu_instance->perf.tlm, &tlm_diff);
//                            PRINT_TIMESPEC(crt_qemu_instance->perf.tlm);

# else
#define START_EXEC_PERF()
#define STOP_EXEC_PERF()
#define START_TRANS_PERF() 
#define STOP_TRANS_PERF()
#define START_TLM_PERF()
#define STOP_TLM_PERF()
#endif

void  qemu_release (qemu_instance *instance);
qemu_cpu_state_t *qemu_get_set_cpu_obj (qemu_instance *instance, unsigned long index, qemu_cpu_wrapper_t *sc_obj);
void  qemu_add_map (qemu_instance *instance, uint32_t base, uint32_t size, int type);
void  qemu_irq_update (qemu_instance *instance, int cpu_mask, int level);
struct qemu_counters_t *qemu_get_counters (qemu_instance *instance);
#ifdef RABBITS_TRACE_EVENT
void qemu_invalidate_address (qemu_instance *instance, uint32_t addr, int src_idx, 
                              tr_event_grp_t type, hwe_cont* hwe_src, uint64_t timestamp);
#else
void qemu_invalidate_address (qemu_instance *instance, uint32_t addr, int src_idx);
#endif


void glue(TARGET_BASE_ARCH_,_generic_machine_init) (int ram_size, 
    const char *cpu_model, int *cpu_cycles);

extern unsigned long    g_crt_no_cycles_instr;
extern unsigned long    g_crt_ns_misses;

#ifdef RABBITS_IMPLEMENT_CACHES
/*#ifdef RABBITS_TRACE_EVENT
void qemu_init_caches (cache_model_t *cache);
#else*/
void qemu_init_caches (void);
/*#endif*/
#endif

void exec_c_init (void);
int flush_orphan_tb (void);
int irq_pending (CPUState *penv);
void just_synchronize (void);
void call_wait_wb_empty (void);
unsigned long get_phys_addr_gdb (unsigned long addr);

#ifdef RABBITS_TRACE_EVENT
qemu_tr_buf_t *qemu_get_tr_buf(qemu_instance *instance, int cpu);
struct qemu_trace_t *qemu_get_set_trace(qemu_instance *instance, uint8_t cmd);
#endif
struct qemu_perf_t *qemu_get_perf(qemu_instance *instance);


#endif
