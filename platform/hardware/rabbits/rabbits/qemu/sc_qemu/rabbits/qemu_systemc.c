#include <time.h>
#include "rabbits/cfg.h"
#include "qemu-common.h"
#include "cpu.h"
#include "hw/irq.h"
#include "rabbits/qemu_encap.h"
#include "rabbits/qemu_systemc.h"
#include "rabbits/save_rest_env.h"
#include "rabbits/trace_power.h"

/*#ifdef RABBITS_TRACE_EVENT
#include <hwetrace.h>
#include <hwetrace_api.h>
#include <events/hwe_device.h>
#endif
*/
unsigned long   g_crt_no_cycles_instr = 0;
unsigned long   g_crt_ns_misses = 0;
#ifdef RABBITS_PERF
//struct timespec tmp_start = {0, 0}, tmp_end = {0, 0}, tmp_diff = {0,0};
struct timespec tlm_start = {0, 0}, tlm_end = {0, 0}, tlm_diff = {0,0};
//struct timespec trans_start = {0, 0}, trans_end = {0, 0}, trans_diff = {0,0};
#endif

bool systemC_enable_trace = false;

static inline uint32_t
qemu_systemc_read_all (void *opaque, target_phys_addr_t addr,
    unsigned char nbytes, int bIO)
{
    uint32_t value = 0xFFFFFFFF;


    if (cpu_single_env->rabbits.tr_id != NULL){
        int insn = cpu_single_env->rabbits.tr_id->inst.body.instr;
        hwe_cont* hwe_src = cpu_single_env->rabbits.tr_id;
        if(insn ==  0xe5963000 || insn ==  0xe580e000){
            printf("\t read_all ");
            printf("[%d.%d]\n", (uint32_t)hwe_src->common.id.devid, (uint32_t)hwe_src->common.id.index);
        }
    }

    addr += (target_phys_addr_t) (long) opaque;

#ifdef RABBITS_TRACE_EVENT
    hwe_cont* hwe_src = cpu_single_env->rabbits.tr_id;
#ifdef RABBITS_TRACE_EVENT_CPU_REQ
    {
    int cpu;
    cpu = cpu_single_env->cpu_index;
    if(hwe_src !=NULL){
    hwe_src = tr_wr_req_event(cpu, hwe_src, addr,
                             TR_EVNT_PROC_IO_REQ, TR_MEM_LOAD);
    }
    }
#endif
#endif

    crt_qemu_instance->m_counters.no_io_read++;
    STOP_EXEC_PERF(); START_TLM_PERF(); 
    SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();

#ifdef RABBITS_TRACE_EVENT
    g_crt_no_cycles_instr = 0;
    if(hwe_src !=NULL){
    _save_crt_qemu_instance->m_systemc.systemc_trace_event(
        _save_cpu_single_env->rabbits.sc_obj);
    }
    value = _save_crt_qemu_instance->m_systemc.systemc_qemu_read_memory (
        _save_cpu_single_env->rabbits.sc_obj, addr, nbytes, bIO, 0);

    // IO MEM ACCESS
#else
    int no_cycles = g_crt_no_cycles_instr;
    g_crt_no_cycles_instr = 0;

    CACHE_LATE_SYNC_MISSES ();
    
    if (no_cycles)
    {
        _save_crt_qemu_instance->m_counters.no_cycles += no_cycles;
        _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
            _save_cpu_single_env->rabbits.sc_obj, no_cycles);
    }
    value = _save_crt_qemu_instance->m_systemc.systemc_qemu_read_memory (
            _save_cpu_single_env->rabbits.sc_obj, addr, nbytes, bIO);
#endif

    RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
//    STOP_TLM_PERF(); START_EXEC_PERF(); 
    START_EXEC_PERF(); STOP_TLM_PERF(); 
    return value;
}

static inline void
qemu_systemc_write_all (void *opaque, target_phys_addr_t addr, uint32_t value,
    unsigned char nbytes, int bIO)
{
    if (cpu_single_env->rabbits.tr_id != NULL){
        int insn = cpu_single_env->rabbits.tr_id->inst.body.instr;
        hwe_cont* hwe_src = cpu_single_env->rabbits.tr_id;
        if(insn ==  0xe5963000 || insn ==  0xe580e000){
            printf("\t write_all ");
            printf("[%d.%d]\n", (uint32_t)hwe_src->common.id.devid, (uint32_t)hwe_src->common.id.index);
        }
    }

    addr += (target_phys_addr_t) (long) opaque;

    if (addr == 0x82000200)
    {
        //sw single instruction
        cpu_single_env->rabbits.sw_single_step = 2 + value;
        return;
    }
// XXX: Event registration was moved to softmmu_template.h file due io_write ...
//      "wrong" calling function

/*#ifdef RABBITS_TRACE_EVENT
    int cpu = cpu_single_env->cpu_index;
    uint32_t src_id = cpu_single_env->rabbits.tr_id;
    src_id = tr_wr_req_event(cpu, src_id, addr,
                             TR_EVNT_PROC_IO_REQ, TR_MEM_STORE);
#endif*/
    crt_qemu_instance->m_counters.no_io_write++;
    STOP_EXEC_PERF(); START_TLM_PERF();
    SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
#ifdef RABBITS_TRACE_EVENT
    g_crt_no_cycles_instr = 0;
    if( _save_cpu_single_env->rabbits.tr_id != NULL){
    _save_crt_qemu_instance->m_systemc.systemc_trace_event(
            _save_cpu_single_env->rabbits.sc_obj);
    }else{
        int no_cycles = g_crt_no_cycles_instr;
        g_crt_no_cycles_instr = 0;
        CACHE_LATE_SYNC_MISSES ();
        if (no_cycles)
        {
            _save_crt_qemu_instance->m_counters.no_cycles += no_cycles;
            _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
                _save_cpu_single_env->rabbits.sc_obj, no_cycles);
        }
    }

    _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
            _save_cpu_single_env->rabbits.sc_obj, addr, value, nbytes, bIO, 0);
#else
    int no_cycles = g_crt_no_cycles_instr;
    g_crt_no_cycles_instr = 0;
    CACHE_LATE_SYNC_MISSES ();
    if (no_cycles)
    {
        _save_crt_qemu_instance->m_counters.no_cycles += no_cycles;
        _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
            _save_cpu_single_env->rabbits.sc_obj, no_cycles);
    }
    _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
        _save_cpu_single_env->rabbits.sc_obj, addr, value, nbytes, bIO);
#endif
    RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
//    STOP_TLM_PERF(); START_EXEC_PERF();
    START_EXEC_PERF(); STOP_TLM_PERF(); 

}

static uint32_t
qemu_systemc_read_b (void *opaque, target_phys_addr_t addr)
{
    return qemu_systemc_read_all (opaque, addr, 1, 1);
}

static void
qemu_systemc_write_b (void *opaque, target_phys_addr_t addr, uint32_t value)
{
     qemu_systemc_write_all (opaque, addr, value, 1, 1);
}

static uint32_t
qemu_systemc_read_w (void *opaque, target_phys_addr_t addr)
{
    return qemu_systemc_read_all (opaque, addr, 2, 1);
}

static void
qemu_systemc_write_w (void *opaque, target_phys_addr_t addr, uint32_t value)
{
    qemu_systemc_write_all (opaque, addr, value, 2, 1);
}

static uint32_t
qemu_systemc_read_dw (void *opaque, target_phys_addr_t addr)
{
    return qemu_systemc_read_all (opaque, addr, 4, 1);
}

static void
qemu_systemc_write_dw (void *opaque, target_phys_addr_t addr, uint32_t value)
{
    qemu_systemc_write_all (opaque, addr, value, 4, 1);
}

static CPUReadMemoryFunc *qemu_systemc_readfn[] = 
{
    qemu_systemc_read_b,
    qemu_systemc_read_w,
    qemu_systemc_read_dw,
};

static CPUWriteMemoryFunc *qemu_systemc_writefn[] = 
{
    qemu_systemc_write_b,
    qemu_systemc_write_w,
    qemu_systemc_write_dw,
};

void
qemu_add_map(qemu_instance *instance, uint32_t base, uint32_t size, int type)
{
    int            iomemtype;
    qemu_instance *save_instance;
	uintptr_t      base_addr = (uintptr_t)base; /* adapt to host address size */

    save_instance = crt_qemu_instance;
    crt_qemu_instance = instance;

    iomemtype = cpu_register_io_memory (
            qemu_systemc_readfn, qemu_systemc_writefn, (void *)base_addr, DEVICE_NATIVE_ENDIAN);
    cpu_register_physical_memory (base, size, iomemtype);

    crt_qemu_instance = save_instance;
}

void 
qemu_release(qemu_instance *instance)
{

	 free(instance->m_io_mem_opaque);
	 free(instance->m_io_mem_read);
	 free(instance->m_io_mem_write);
	 free(instance->m_io_mem_used);
	 free(instance);

}

qemu_cpu_state_t *
qemu_get_set_cpu_obj(qemu_instance *instance, unsigned long index,
					 qemu_cpu_wrapper_t *sc_obj)
{
    qemu_instance       *save_instance;
    CPUState         	*penv;
    int                 i;

    save_instance = crt_qemu_instance;
    crt_qemu_instance = instance;
    penv = (CPUState *) crt_qemu_instance->m_first_cpu;

    for (i = 0; i < index; i++)
        penv = penv->next_cpu;

    penv->rabbits.sc_obj = sc_obj;

    crt_qemu_instance = save_instance;

    return penv;
}

void qemu_irq_update (qemu_instance *instance, int cpu_mask, int level)
{
    if (!instance || !instance->irqs_systemc)
        return;

    int cpu;

    for (cpu = 0; cpu < instance->m_NOCPUs; cpu++)
    {
        if (cpu_mask & (1 << cpu))
        {
            if (instance->irqs_systemc[cpu] == NULL)
                return;

            qemu_set_irq ((qemu_irq) instance->irqs_systemc[cpu], level);
        }
    }
}

struct qemu_counters_t *qemu_get_counters (qemu_instance *instance)
{
    return &instance->m_counters;
}

void just_synchronize (void)
{
    STOP_EXEC_PERF(); START_TLM_PERF();
    SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
    int no_cycles = g_crt_no_cycles_instr;
    g_crt_no_cycles_instr = 0;
#ifndef RABBITS_TRACE_EVENT
    CACHE_LATE_SYNC_MISSES ();
#endif
    if (no_cycles)
    {
        _save_crt_qemu_instance->m_counters.no_cycles += no_cycles;
#ifdef RABBITS_TRACE_EVENT
        if( _save_cpu_single_env->rabbits.tr_id != NULL){
        _save_crt_qemu_instance->m_systemc.systemc_trace_event(
            _save_cpu_single_env->rabbits.sc_obj);
        }else{

            _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
                _save_cpu_single_env->rabbits.sc_obj, no_cycles);
        }
#else
        _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
            _save_cpu_single_env->rabbits.sc_obj, no_cycles);
#endif
    }
    RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
//    STOP_TLM_PERF() ; START_EXEC_PERF();
    START_EXEC_PERF(); STOP_TLM_PERF(); 
}

int flush_orphan_tb (void)
{
    if (cpu_single_env->rabbits.flush_last_tb == NULL)
        return 0;

    TranslationBlock    *tb = cpu_single_env->rabbits.flush_last_tb;
    int                 tb_idx = ((unsigned long) tb -
        (unsigned long) crt_qemu_instance->m_tbs) / sizeof (TranslationBlock);

    cpu_single_env->rabbits.flush_last_tb = NULL;

    if (!cpu_single_env->rabbits.need_flush)
        return 0;

    if (!tb->rabbits.flush_cnt)
    {
        printf ("%s: env->need_flush=%d, tb->flush_cnt=%d, "
                "cpu=%d, tb=%d\n",
            __FUNCTION__, cpu_single_env->rabbits.need_flush,
            tb->rabbits.flush_cnt, cpu_single_env->cpu_index,
            tb_idx);
        exit (1);
    }
    if (tb_idx != cpu_single_env->rabbits.flush_idx_blocked_tb)
    {
        printf ("%s: tb blocked=%d, tb freed=%d!\n",
            __FUNCTION__, cpu_single_env->rabbits.flush_idx_blocked_tb, tb_idx);
        exit (1);
    }

    DTBALLOCPRINTF ("FREE cpu=%d, tb=%d.pc=0x%lx, cnt=%d\n",
        cpu_single_env->cpu_index,
        tb_idx,
        (unsigned long) tb->pc, tb->rabbits.flush_cnt);

    #if 0
    if (cpu_single_env->cpu_index==0 && tb_idx==143 && tb->pc==0xc00605f8)
    {
        printf ("the err point\n");
    }
    #endif

    cpu_single_env->rabbits.need_flush = 0;
    cpu_single_env->rabbits.flush_idx_blocked_tb = -1;
    tb->rabbits.flush_cnt--;

    if (tb->rabbits.flush_cnt == 0)
    {
        TranslationBlock    **ptb;
        ptb = (struct TranslationBlock **) &crt_qemu_instance->m_flush_head;
        while (*ptb && *ptb != tb)
            ptb = & (*ptb)->rabbits.flush_next;
        if (!*ptb)
        {
            printf ("%s: cannot find TB in the flushing list!\n", __FUNCTION__);
            exit (1);
        }
        *ptb = tb->rabbits.flush_next;
        tb->rabbits.flush_next = NULL;
    }
    
    return 1;
}

#ifdef RABBITS_IMPLEMENT_CACHES
/*#ifdef RABBITS_TRACE_EVENT
void
qemu_init_caches (cache_model_t *cache)
{
    int line, cpu, w;

    uint32_t d_lines = cache->data.lines;
    uint32_t d_line_bytes = 1 << cache->data.line_bits;
    uint32_t d_line_words = 1 << (cache->data.line_bits - 2);

    uint32_t i_lines      = cache->instr.lines;
    uint32_t i_line_bytes = 1 << cache->instr.line_bits;
    uint32_t i_line_words = 1 << (cache->instr.line_bits -2);

    START_EXEC_PERF();

    crt_qemu_instance->m_cpu_dcache =
                  malloc(crt_qemu_instance->m_NOCPUs * sizeof(unsigned long *));
    for (cpu = 0; cpu < crt_qemu_instance->m_NOCPUs; cpu++){
        crt_qemu_instance->m_cpu_dcache[cpu] = malloc (d_lines * sizeof(long));
        for (line = 0; line < d_lines; line++){
            crt_qemu_instance->m_cpu_dcache[cpu][line] = (unsigned long) -1;
        }
    }
    crt_qemu_instance->m_cpu_icache =
                  malloc(crt_qemu_instance->m_NOCPUs * sizeof(unsigned long *));
    for (cpu = 0; cpu < crt_qemu_instance->m_NOCPUs; cpu++){
        crt_qemu_instance->m_cpu_icache[cpu] = malloc (i_lines * sizeof(long));
        for (line = 0; line < i_lines; line++){
            crt_qemu_instance->m_cpu_icache[cpu][line] = (unsigned long) -1;
        }
    }
    crt_qemu_instance->m_cpu_dcache_data =
                          malloc(crt_qemu_instance->m_NOCPUs * sizeof (char**));
    for (cpu = 0; cpu < crt_qemu_instance->m_NOCPUs; cpu++){
        crt_qemu_instance->m_cpu_dcache_data[cpu] =
                          malloc(d_lines * sizeof(char*));
        for (line = 0; line < d_lines; line++){
            crt_qemu_instance->m_cpu_dcache_data[cpu][line] =
                          malloc( d_line_bytes * sizeof(char));
            for (w = 0; w < d_line_words; w++){
                ((uint32_t *) crt_qemu_instance->m_cpu_dcache_data[cpu][line])[w] =
                    (uint32_t) 0xDEADBEAF;
            }
        }
    }
    crt_qemu_instance->m_cpu_icache_data =
                          malloc (crt_qemu_instance->m_NOCPUs* sizeof(char**));
    for (cpu = 0; cpu < crt_qemu_instance->m_NOCPUs; cpu++){
    crt_qemu_instance->m_cpu_icache_data[cpu] =
                          malloc (i_lines * sizeof(char*));
        for (line = 0; line < i_lines; line++){
            crt_qemu_instance->m_cpu_icache_data[cpu][line] =
                          malloc ( i_line_bytes * sizeof(char));
            for (w = 0; w < i_line_words; w++){
                ((uint32_t *) crt_qemu_instance->m_cpu_icache_data[cpu][line])[w] =
                    (uint32_t) 0xDEADBEAF;
            }
        }
    }
}
#else*/
void
qemu_init_caches (void)
{
    int line, cpu, w;

    crt_qemu_instance->m_cpu_dcache_tag = malloc(crt_qemu_instance->m_NOCPUs *
        DCACHE_LINES * sizeof (unsigned long));
    /* Not really useful but may trigger an error in case of illegal access */
    for (cpu = 0; cpu < crt_qemu_instance->m_NOCPUs; cpu++)
        for (line = 0; line < DCACHE_LINES; line++)
            crt_qemu_instance->m_cpu_dcache_tag[cpu][line] = 0x8BADF00D;

    crt_qemu_instance->m_cpu_dcache_flags = malloc(crt_qemu_instance->m_NOCPUs *
        DCACHE_LINES * sizeof (uint8_t));
    for (cpu = 0; cpu < crt_qemu_instance->m_NOCPUs; cpu++)
        for (line = 0; line < DCACHE_LINES; line++) {
            crt_qemu_instance->m_cpu_dcache_flags[cpu][line].valid = 0;
            crt_qemu_instance->m_cpu_dcache_flags[cpu][line].dirty = 0;
            crt_qemu_instance->m_cpu_dcache_flags[cpu][line].state = 0;
        }

    crt_qemu_instance->m_cpu_dcache_data = malloc(crt_qemu_instance->m_NOCPUs *
        DCACHE_LINES * DCACHE_LINE_BYTES * sizeof (unsigned char));
    for (cpu = 0; cpu < crt_qemu_instance->m_NOCPUs; cpu++)
        for (line = 0; line < DCACHE_LINES; line++)
            for (w = 0; w < DCACHE_LINE_WORDS; w++)
                ((uint32_t *)crt_qemu_instance->m_cpu_dcache_data[cpu][line])[w] =
                    (uint32_t)0xDEADBEEF;

    crt_qemu_instance->m_cpu_icache_tag = malloc(crt_qemu_instance->m_NOCPUs *
        ICACHE_LINES * sizeof (unsigned long));
    for (cpu = 0; cpu < crt_qemu_instance->m_NOCPUs; cpu++)
        for (line = 0; line < ICACHE_LINES; line++)
            crt_qemu_instance->m_cpu_icache_tag[cpu][line] = 0x8BADF00D;

    crt_qemu_instance->m_cpu_icache_flags = malloc(crt_qemu_instance->m_NOCPUs *
        DCACHE_LINES * sizeof (uint8_t));
    for (cpu = 0; cpu < crt_qemu_instance->m_NOCPUs; cpu++)
        for (line = 0; line < DCACHE_LINES; line++) {
            crt_qemu_instance->m_cpu_dcache_flags[cpu][line].valid = 0;
        }

    crt_qemu_instance->m_cpu_icache_data = malloc(crt_qemu_instance->m_NOCPUs *
        ICACHE_LINES * ICACHE_LINE_BYTES * sizeof (unsigned char));
    for (cpu = 0; cpu < crt_qemu_instance->m_NOCPUs; cpu++)
        for (line = 0; line < ICACHE_LINES; line++)
            for (w = 0; w < ICACHE_LINE_WORDS; w++)
                ((uint32_t *)crt_qemu_instance->m_cpu_icache_data[cpu][line])[w] =
                    (uint32_t)0xDEADBEEF;

}
/*#endif*/

void call_wait_wb_empty (void)
{
    STOP_EXEC_PERF(); START_TLM_PERF();
    SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();

#ifdef RABBITS_TRACE_EVENT
    if( _save_cpu_single_env->rabbits.tr_id != NULL){
    _save_crt_qemu_instance->m_systemc.systemc_trace_event(
        _save_cpu_single_env->rabbits.sc_obj);
    }
#endif
    _save_crt_qemu_instance->m_systemc.wait_wb_empty (
        _save_cpu_single_env->rabbits.sc_obj);
    RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
//    STOP_TLM_PERF(); START_EXEC_PERF(); 
    START_EXEC_PERF(); STOP_TLM_PERF(); 
}
#endif


#ifdef RABBITS_TRACE_EVENT
qemu_tr_buf_t *qemu_get_tr_buf(qemu_instance *instance, int cpu)
{
    return &instance->m_tr_buf[cpu];
}

struct qemu_trace_t *qemu_get_set_trace(qemu_instance *instance, uint8_t cmd)
{
    if (cmd){ // WRITE
        printf("QEMU: SET_SET_TRACE\n");
        instance->trace.enable = true;
        systemC_enable_trace = true;
    } else {//READ
        printf("QEMU: GET_SET_TRACE\n");
    }

    return &instance->trace;
}


#endif

//#ifdef RABBITS_PERF
struct qemu_perf_t *qemu_get_perf(qemu_instance *instance)
{
// printf("T%lld.%.9ld\n", (long long)instance->perf.translation.tv_sec, instance->perf.translation.tv_nsec);
// printf("E%lld.%.9ld\n", (long long)instance->perf.execution.tv_sec, instance->perf.execution.tv_nsec);
// printf("M%lld.%.9ld\n", (long long)instance->perf.tlm.tv_sec, instance->perf.tlm.tv_nsec);
   return &instance->perf;
}

//#endif

