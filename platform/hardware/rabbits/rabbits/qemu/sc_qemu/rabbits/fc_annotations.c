/**
 * @file
 * @author ...
 * @version 1.0
 *
 * @section LICENSE
 *
 * ...
 *
 * @section DESCRIPTION
 *
 * ...
 */

/*
 * vim:sw=4:tw=0:
 */
#include "rabbits/cfg.h"
#include "qemu-common.h"
#include "cpu.h"
#include "rabbits/fc_annotations.h"
#include "rabbits/qemu_systemc.h"
#include "rabbits/gdb_srv.h"
#include "rabbits/save_rest_env.h"

#ifdef RABBITS_TRACE_EVENT
#include "rabbits/trace_power.h"
#include <hwetrace.h>
#include <hwetrace_api.h>
#include <hwetrace_common.h>
#include <hwetrace_cache.h>
#include <hwetrace_processor.h>
#include <events/hwe_device.h>
#endif

#ifdef  RABBITS_PERF
#include <time.h>
#endif

extern int          dcache_line_bits;
extern int          icache_line_bits;

extern int          dcache_assoc_bits;
extern int          icache_assoc_bits;

extern target_ulong dcache_line_mask;
extern target_ulong icache_line_mask;

extern int          dcache_lines;
extern int          icache_lines;

extern int          dcache_line_bytes;

#ifdef RABBITS_PERF
extern struct timespec tmp_start, tmp_end, tmp_diff;
extern struct timespec tlm_start, tlm_end, tlm_diff;
extern struct timespec trans_start, trans_end, trans_diff;
#endif

bool next_addr = false;
int ni = 0;

//#define DEBUG_TRACE_GEN

#ifdef DEBUG_TRACE_GEN
#define PRINTEVNT(str,e,d,i) if((e->common.id.devid == d && e->common.id.index == i) || (i == 0 && d == 0)){ \
                                 printf("%s [EVENT] [%d.%ld] 0x%08x inst 0x%08x\n", str,\
                                         e->common.id.devid, (uint64_t)e->common.id.index, e->inst.body.pc, e->inst.body.instr);}
#else
#define PRINTEVNT(str,e,d,i)
#endif
/*
 * FIXME: Work to be done on the cache model, which is currently a write through update physically addressed cache.
 *        - use target_phys_addr_t for the addresses and tag types
 *        - use target_ulong as type for the data within the cache
 *        - update the cache model to support 8 bytes access if TARGET_LONG_SIZE == 8
 */

void tb_start (TranslationBlock *tb)
{
    cpu_single_env->rabbits.flush_last_tb = tb;

    if (g_crt_no_cycles_instr > 2000)
    {
//        printf("[%d] Just_syncronize\n", cpu_single_env->cpu_index);
        just_synchronize ();
    }
}
#ifdef RABBITS_IMPLEMENT_CACHES

/** Data cache function
 *
 * @param cpu cpu number [from 0 to max number of cpu system]
 * @param start_idx start index of cache memory
 * @param tag Line of instruction
 *
 * @return -1 Cache MISS, otherwise cache HIT
 */
static inline int dcache_line_present (int cpu, int start_idx, unsigned long tag)
{
    int idx, last_idx = start_idx + (1 << dcache_assoc_bits);
    for (idx = start_idx; idx < last_idx; idx++)
        if (tag == crt_qemu_instance->m_cpu_dcache_tag[cpu][idx] &&
            crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid )
            return idx;

    return -1;
}

static inline int dcache_line_replace (int cpu, int start_idx)
{
    int idx, last_idx = start_idx + (1 << dcache_assoc_bits);
    for (idx = start_idx; idx < last_idx; idx++)
        if (crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid == 0)
            return idx;

    return start_idx + (((1 << dcache_assoc_bits) - 1) & random ());
}

/** Instruction cache function
 *
 * @param cpu cpu number [from 0 to max number of cpu system]
 * @param start_idx start index of cache memory
 * @param tag Line of instruction
 *
 * @return -1 Cache MISS, otherwise cache HIT
 */
static inline int icache_line_present (int cpu, int start_idx, unsigned long tag)
{
    int idx, last_idx = start_idx + (1 << icache_assoc_bits);
    for (idx = start_idx; idx < last_idx; idx++)
        if (/*tag == crt_qemu_instance->m_cpu_icache_tag[cpu][idx] &&*/
            crt_qemu_instance->m_cpu_icache_flags[cpu][idx].valid)
            return idx;

    return -1;
}

static inline int icache_line_replace (int cpu, int start_idx)
{
    int idx, last_idx = start_idx + (1 << icache_assoc_bits);
    for (idx = start_idx; idx < last_idx; idx++)
        if (crt_qemu_instance->m_cpu_icache_flags[cpu][idx].valid == 0)
            return idx;

    return start_idx + (((1 << icache_assoc_bits) - 1) & random ());
}

static inline void *get_host_address (unsigned long addr)
{
    #ifdef ONE_MEMORY_MODULE
    return (void *) (addr + cpu_single_env->rabbits.sc_mem_host_addr);
    #else //!ONE_MEMORY_MODULE
    return (void *)crt_qemu_instance->m_systemc.systemc_get_mem_addr (
            cpu_single_env->rabbits.sc_obj, crt_qemu_instance->m_systemc.subsystem, addr);
    #endif //ONE_MEMORY_MODULE
}

//#define ADDR_DBG 0x00033fb8
#define ADDR_DBG 0x1


#define VERBOSE_CACHE_OPERATIONS


/**
 * @brief 
 */
void
dcache_invalidate_all(void)
{
    int i, cpu = cpu_single_env->cpu_index;
    // a memset will be more performante
    printf("EXE: [%d] Invalidating all...\n", cpu);
    for(i = 0; i < dcache_lines; i++) {
        crt_qemu_instance->m_cpu_dcache_flags[cpu][i].valid = (unsigned long) 0;
    }
}

/**
 * Invalidate line caches through instruction
 *
 * @param addr
 * @return void
 */
void
dcache_invalidate(unsigned long addr)
{
#if 1
#ifdef IMPLEMENT_FULL_CACHES
    unsigned long dtag = addr >> dcache_line_bits;
    int           didx, dstart_idx = dtag & (dcache_lines - 1) &
                                ~((1 << dcache_assoc_bits) - 1);
    qemu_instance *old_instance = crt_qemu_instance;
    int cpu = cpu_single_env->cpu_index;


    didx = dcache_line_present (cpu, dstart_idx, dtag);

    if (didx != -1){
        if((addr & ~dcache_line_mask) == (ADDR_DBG & ~dcache_line_mask)){
            printf("INVAL [  0x%08x - 0x%08x  ]\tby cpu %d\n",(int)(addr & ~dcache_line_mask), (int)(addr & ~dcache_line_mask)+dcache_line_bytes-1, cpu);
        }
#ifdef RABBITS_TRACE_EVENT
        hwe_cont* hwe_src = cpu_single_env->rabbits.tr_id;
        tr_wr_invalidate_event(cpu, addr & ~dcache_line_mask, TR_EVNT_DCACHE_INV,hwe_src, 0);
#endif
        crt_qemu_instance->m_cpu_dcache_flags[cpu][didx].valid = (unsigned long) 0;
    }
    crt_qemu_instance = old_instance;
#endif /* IMPLEMENT_FULL_CACHES */
#else
#endif
}

void
dcache_flush_all(void)
{
#if defined(IMPLEMENT_FULL_CACHES) && defined(CACHE_IS_WB)
    int cpu, i,j =0;
    cpu = cpu_single_env->cpu_index;
    hwe_cont* hwe_proc = cpu_single_env->rabbits.tr_id;

    printf("EXE: [%d] Flushing all...\n",cpu);
    for(i = 0; i < dcache_lines; i++){ 
        if ((crt_qemu_instance->m_cpu_dcache_flags[cpu][i].valid == 1) &&
            (crt_qemu_instance->m_cpu_dcache_flags[cpu][i].dirty == 1)) {
            unsigned long cached_tag = crt_qemu_instance->m_cpu_dcache_tag[cpu][i];
            unsigned long cached_addr = cached_tag << DCACHE_LINE_BITS;
            hwe_cont* hwe_src;
          if(cached_addr == (ADDR_DBG & ~dcache_line_mask)){
              printf("FLUSH [0x%08x-0x%08x]\t by cpu %d linex %d index %d\n",(uint32_t) (cached_addr),(uint32_t) (cached_addr)+dcache_line_bytes-1,cpu, i, j++);
          }
            hwe_src = tr_wr_str_event(cpu, hwe_proc, cached_addr,
                    TR_EVNT_DCACHE_WRITEBACK, dcache_line_bytes);
            
            crt_qemu_instance->m_cpu_dcache_flags[cpu][i].dirty = 0;

            /* Actual update of the memory using a backdoor access */
            memcpy (get_host_address(cached_addr),
                    crt_qemu_instance->m_cpu_dcache_data[cpu][i],
                    dcache_line_bytes);
            SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
#ifdef RABBITS_TRACE_EVENT
            _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
                    _save_cpu_single_env->rabbits.sc_obj, cached_addr,
                    _save_crt_qemu_instance->m_cpu_dcache_data[cpu][i][0], 1, 0,hwe_src);
#else
            _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
                    _save_cpu_single_env->rabbits.sc_obj, cached_addr,
                    _save_crt_qemu_instance->m_cpu_dcache_data[cpu][i][0], 1, 0);
#endif
            RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
        }
    }
#endif /* IMPLEMENT_FULL_CACHES */

}



/**
 * Flush (clean) line cache through instruction
 *
 * @param addr
 * @return void
 */
void
dcache_flush(unsigned long addr)
{
#ifdef IMPLEMENT_FULL_CACHES
	 /* TODO: Flush line containing addr from the current cache.  */
	 /* Write modified data to memory and invalidate line.        */
	 /* Internally only. No need to propagate to other processors */
#endif /* IMPLEMENT_FULL_CACHES */

#if defined(IMPLEMENT_FULL_CACHES) && defined(CACHE_IS_WB)
    int cpu, idx, start_idx;
    unsigned long tag;

    cpu = cpu_single_env->cpu_index;
    tag = addr >> dcache_line_bits;
    start_idx = tag & (dcache_lines - 1) & ~((1 << dcache_assoc_bits) - 1);
    idx = dcache_line_present(cpu, start_idx, tag);

    if (idx != -1 && (crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty == 1)) {
        hwe_cont* hwe_src;
        if((addr & ~dcache_line_mask) == (ADDR_DBG & ~dcache_line_mask)){
            printf("FLUSH [  0x%08x - 0x%08x  ]\tby cpu %d valid %d dirty %d pc 0x%08x inst 0x%08x\n",(uint32_t) (addr & ~dcache_line_mask),
                    (uint32_t) (addr & ~dcache_line_mask)+dcache_line_bytes-1,cpu,
                    crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid, crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty,
                    cpu_single_env->rabbits.tr_id->inst.body.pc,
                    cpu_single_env->rabbits.tr_id->inst.body.instr);

        }
        hwe_src = tr_wr_str_event(cpu, cpu_single_env->rabbits.tr_id, addr & ~dcache_line_mask,
                                  TR_EVNT_DCACHE_WRITEBACK, dcache_line_bytes);

        /* Actual update of the memory using a backdoor access */
        memcpy (get_host_address(addr & ~dcache_line_mask),
                crt_qemu_instance->m_cpu_dcache_data[cpu][idx],
                dcache_line_bytes);
        SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
        #ifdef RABBITS_TRACE_EVENT
        if(_save_cpu_single_env->rabbits.tr_id != NULL){ // trace enabled 
            _save_crt_qemu_instance->m_systemc.systemc_trace_event(
                    _save_cpu_single_env->rabbits.sc_obj);
        }
        _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
                _save_cpu_single_env->rabbits.sc_obj, addr & ~DCACHE_LINE_MASK,
                _save_crt_qemu_instance->m_cpu_dcache_data[cpu][idx][0], 1, 0,hwe_src);
        #else
        _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
                _save_cpu_single_env->rabbits.sc_obj, addr & ~DCACHE_LINE_MASK,
                _save_crt_qemu_instance->m_cpu_dcache_data[cpu][idx][0], 1, 0);
        #endif
        RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
        crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty = 0;
    }
#endif /* IMPLEMENT_FULL_CACHES */

}

/**
 * Data cache read
 *
 * @param addr Read Memory Cache Address
 *
 * @return
=======

 * Helper to read from dcache, fills up the cache line on miss.
 * SystemC read access is cache line aligned, so we have no hardware alignemnt problem
 */
void * REGPARM
dcache_read(unsigned long addr)
{
    if (!cpu_single_env || cpu_single_env->rabbits.b_use_backdoor) {
        fprintf (stderr, "Error in %s, env=0x%lx, backdoor=%d\n",
            __FUNCTION__, (unsigned long) cpu_single_env,
            cpu_single_env ? (int) cpu_single_env->rabbits.b_use_backdoor : 0);
        exit (1);
    }

    #ifdef IMPLEMENT_FULL_CACHES
    int no_cycles = g_crt_no_cycles_instr;
        g_crt_no_cycles_instr = 0;
        crt_qemu_instance->m_counters.no_cycles += no_cycles;
        STOP_EXEC_PERF(); START_TLM_PERF();
        SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
        #ifdef RABBITS_TRACE_EVENT
        if(_save_cpu_single_env->rabbits.tr_id != NULL){ // trace enabled 
        // XXX: MUST BE HERE WITHOUT no_cycles TEST (To maintain the events orders)
            _save_crt_qemu_instance->m_systemc.systemc_trace_event(
                _save_cpu_single_env->rabbits.sc_obj);
        }else if(no_cycles > 0){
            _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
                _save_cpu_single_env->rabbits.sc_obj, no_cycles);
        }
        #else
        if (no_cycles > 0){
        _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
            _save_cpu_single_env->rabbits.sc_obj, no_cycles);
        }
        #endif
        RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
        START_EXEC_PERF(); STOP_TLM_PERF(); 
    #endif //IMPLEMENT_FULL_CACHES

    int cpu, idx, start_idx;
    unsigned long tag;

    #ifdef RABBITS_TRACE_EVENT
    unsigned long tag_inval;
    hwe_cont* hwe_src   = cpu_single_env->rabbits.tr_id;
    uint8_t *read_size = &cpu_single_env->rabbits.read_size; 
    #endif

    cpu = cpu_single_env->cpu_index;
    tag = addr >> dcache_line_bits;
    start_idx = tag & (dcache_lines - 1) & ~((1 << dcache_assoc_bits) - 1);
    idx = dcache_line_present (cpu, start_idx, tag);

    #ifdef RABBITS_TRACE_EVENT
    #ifdef RABBITS_TRACE_EVENT_CPU_REQ   
    hwe_src = tr_wr_req_event(cpu, hwe_src, addr,
                              TR_EVNT_PROC_DCACHE_REQ, TR_MEM_LOAD);
    PRINTEVNT("R ",hwe_src,0,0);

    #endif
    #endif

    if (idx == -1){  // CACHE_MISS

        crt_qemu_instance->m_counters.no_dcache_miss++;

        idx = dcache_line_replace (cpu, start_idx);
        #ifdef RABBITS_TRACE_EVENT
        tag_inval =  crt_qemu_instance->m_cpu_dcache_tag[cpu][idx];
        if (tag_inval != -1){
            tr_wr_invalidate_event(cpu, tag_inval << dcache_line_bits ,
                    TR_EVNT_DCACHE_REPL, cpu_single_env->rabbits.tr_id, 0);
        }
        #endif

         if((tag_inval << DCACHE_LINE_BITS)  == (ADDR_DBG & ~dcache_line_mask)) {
             printf("EVICT [A 0x%08x C 0x%08x  ]\tby cpu %d valid %d dirty %d pc 0x%08x inst 0x%08x\n",(int)addr&~dcache_line_mask,
                     (int)(tag_inval << DCACHE_LINE_BITS), cpu, 
                     crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid, crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty,
                     cpu_single_env->rabbits.tr_id->inst.body.pc,
                     cpu_single_env->rabbits.tr_id->inst.body.instr);

         }
#ifdef CACHE_IS_WB
        /* This is the victim we have to evict, if valid and dirty, copy back to memory, otherwise, do nothing */
        if (crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid == 1 &&
            crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty == 1) {
            /* Rebuild the address in which to copy the data back : hopefully tag contains exactly this */
            unsigned long cached_tag = crt_qemu_instance->m_cpu_dcache_tag[cpu][idx];
            unsigned long cached_addr = cached_tag << DCACHE_LINE_BITS;
            /* Actual update of the memory using a backdoor access */
            /* Very tricky stuff, ... */
            #ifdef RABBITS_TRACE_EVENT
            if(cached_addr == (ADDR_DBG & ~dcache_line_mask)){
                printf("WBACK [  0x%08x - 0x%08x  ]\tby cpu %d\n",(uint32_t) (cached_addr & ~dcache_line_mask),(uint32_t) (addr & ~dcache_line_mask)+dcache_line_bytes-1, cpu);
          }

            hwe_src = tr_wr_str_event(cpu, cpu_single_env->rabbits.tr_id, cached_addr,
                                      TR_EVNT_DCACHE_WRITEBACK, dcache_line_bytes);
            PRINTEVNT("WB",hwe_src,0,0);
            #endif
            memcpy (get_host_address(cached_addr),
                    crt_qemu_instance->m_cpu_dcache_data[cpu][idx],
                    DCACHE_LINE_BYTES);
            SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
            if(_save_cpu_single_env->rabbits.tr_id != NULL){ // Trace Enabled
                g_crt_no_cycles_instr = 0;
                _save_crt_qemu_instance->m_systemc.systemc_trace_event(
                        _save_cpu_single_env->rabbits.sc_obj); // TRACE BUFFER
            }
            _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
                _save_cpu_single_env->rabbits.sc_obj, cached_addr, _save_crt_qemu_instance->m_cpu_dcache_data[cpu][idx][0], 1, 0,hwe_src);
            RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
        }
     #endif
       //Verify if it is a write-allocate 
        #ifdef RABBITS_TRACE_EVENT
        hwe_src = tr_wr_req_event(cpu, cpu_single_env->rabbits.tr_id, addr & ~dcache_line_mask,
                                   TR_EVNT_DCACHE_REQ, TR_MEM_LOAD);
        PRINTEVNT("RC",hwe_src,0,0);
       
        if(cpu_single_env->rabbits.excl){
            printf("EX: READ MISS\n");
            if(cpu_single_env->rabbits.tr_id->inst.body.str){
                printf("EX: ALLOC\n");
            }
        }

        if(hwe_src != NULL){
            if(*read_size != 0 && !cpu_single_env->rabbits.tr_id->inst.body.str){
                HWE_CACHE_set_cpureq(hwe_src, addr, *read_size);
//                printf(" Error addr = 0x%08x read_size = %d\n",(unsigned int) addr,(unsigned int) *read_size);
                *read_size = 0;
            }else{
                HWE_CACHE_set_cpureq(hwe_src, 0, 0);
            }
        }
        #endif

        #ifdef IMPLEMENT_FULL_CACHES
        STOP_EXEC_PERF(); START_TLM_PERF();
        SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
        #ifdef RABBITS_TRACE_EVENT
        if(hwe_src != NULL){ 
            _save_crt_qemu_instance->m_systemc.systemc_trace_event(
                _save_cpu_single_env->rabbits.sc_obj); // consume the events
        }
        _save_crt_qemu_instance->m_systemc.systemc_qemu_read_memory (
                    _save_cpu_single_env->rabbits.sc_obj, addr & ~dcache_line_mask,
                    1 << dcache_line_bits, 0, hwe_src); // read memory with
                                                        // trace information
        #else
        _save_crt_qemu_instance->m_systemc.systemc_qemu_read_memory (
            _save_cpu_single_env->rabbits.sc_obj, addr & ~dcache_line_mask,
            1 << dcache_line_bits, 0);
        #endif
        RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
        START_EXEC_PERF(); STOP_TLM_PERF(); 

        crt_qemu_instance->m_cpu_dcache_tag[cpu][idx]         = tag;
        crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid = 1;
#ifdef CACHE_IS_WB
        crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty = 0;
#endif
        if(addr == ADDR_DBG){
            printf("RMISS [  0x%08x   size %05d  ]\tby cpu %d valid %d dirty %d\n",(uint32_t) (addr),(uint32_t)*read_size,cpu,
                    crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid, crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty);
        }

        memcpy (crt_qemu_instance->m_cpu_dcache_data[cpu][idx],
            get_host_address (addr & ~dcache_line_mask), dcache_line_bytes);
        #else //IMPLEMENT_LATE_CACHES
        g_crt_ns_misses += NS_DCACHE_MISS;
        #endif //IMPLEMENT_LATE_CACHES
        
    } else { // CACHE HIT
#ifdef RABBITS_TRACE_EVENT
#ifdef RABBITS_TRACE_EVENT_CACHE
        if(cpu_single_env->rabbits.excl){
            printf("EX: READ HIT\n");
            if(cpu_single_env->rabbits.tr_id->inst.body.str){
                printf("EX: ALLOC\n");
            }
        }
        if(addr == ADDR_DBG){
            printf("RHIT  [  0x%08x   size %05d  ]\tby cpu %d valid %d dirty %d\n",(uint32_t) (addr),(uint32_t)*read_size,cpu,
                    crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid, crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty);
        }


        if( *read_size != 0){
            hwe_src = tr_wr_ack_event(cpu, hwe_src, addr, *read_size, TR_EVNT_DCACHE_ACK, TR_MEM_LOAD);
            PRINTEVNT("RC",hwe_src,0,0);
        }else{
            printf("read_size error: %d\n",*read_size);
            exit(1);
        }
        *read_size = 0;
#endif
#endif
    }

    #ifdef RABBITS_GDB_ENABLED
    if(crt_qemu_instance->m_gdb)
    {
    int                 i, nb = crt_qemu_instance->m_gdb->watchpoints.nb;
    struct watch_el_t   *pwatch = crt_qemu_instance->m_gdb->watchpoints.watch;

    for (i = 0; i < nb; i++)
        if (addr >= pwatch[i].begin_address && addr < pwatch[i].end_address &&
            (pwatch[i].type == GDB_WATCHPOINT_READ || pwatch[i].type == GDB_WATCHPOINT_ACCESS)
            )
        {
            gdb_loop (i, 0, 0);
            break;
        }
    }
    #endif //RABBITS_GDB_ENABLED

    #ifdef IMPLEMENT_FULL_CACHES
    return &crt_qemu_instance->m_cpu_dcache_data[cpu][idx][addr & dcache_line_mask];
    #else //!IMPLEMENT_FULL_CACHES
    return get_host_address (addr);
    #endif //IMPLEMENT_FULL_CACHES
//    STOP_EXEC_PERF();
}

void * REGPARM
dcache_read_direct(unsigned long addr, unsigned int size)
{
#if 0
    if(cpu_single_env->rabbits.tr_id != NULL){
        if((addr >= ADDR_DBG) &&
                (addr <= (ADDR_DBG+3))){
            printf("@@@@@ 1 here DIRECT addr = 0x%08x @@@@@ m_size %d size %x @0x%08x!!!\n",(int)addr,1 << (size & 3), size, cpu_single_env->rabbits.tr_id->inst.body.pc);
        }

        if((addr & ~0x1FUL) == (ADDR_DBG & ~0x1FUL)){
            printf("@@@@@ 2 here DIRECT addr = 0x%08x @@@@@  m_size %d size %x @0x%08x!!!\n",(int)addr, 1 << (size & 3),size,cpu_single_env->rabbits.tr_id->inst.body.pc);
        }
    }
#endif
    #ifdef RABBITS_TRACE_EVENT
    cpu_single_env->rabbits.read_size = 1 << (size & 3);
    #endif
#if 0
    fprintf(stderr, "@@@@@@ %s(0x%08ld,%d,%d)\n", __func__, addr, size, cpu_single_env->rabbits.read_size);
#endif
    return dcache_read(addr);
}

/**
 * Data cache read long long
 *
 * @param addr Read address
 *
 * @return
 */

/*
 * Cache access needs to handle unaligned loads, which complexifies a bit the thing
 * I assume that all read accesses can be unaligned, even though doublewords are
 * only used for ldrexd which *must* be aligned on ARM, but if we reuse this for
 * other architectures, we may need to support looser constraints.
 *
 * Note that read and write are *not* symmetrical: indeed, when reading the access to
 * the hardware is done with a cache line granularity (and alignment), whereas when
 * writting we *must* be aligned
 *
 * FIXME: use the qemu types instead of the old school original K&R C types
 */

unsigned long long
dcache_read_q (unsigned long addr)
{
    unsigned long   low, hi;

    low = dcache_read_l(addr);
    hi  = dcache_read_l(addr + 4);
   
    return (((unsigned long long) hi) << 32) + low;
}

/**
 * Data cache read long
 *
 * @param addr Read address
 *
 * @return
 */
unsigned long
dcache_read_l(unsigned long addr)
{
    if((addr >= ADDR_DBG) &&
            (addr <= (ADDR_DBG+3)))
        printf("@@@@@ here L @@@@@ !!!\n");

    if ((addr & 3) == 0){ /* Aligned word access, normal case, lets do it now and fast */
        #ifdef RABBITS_TRACE_EVENT
        cpu_single_env->rabbits.read_size = 4; 
        #endif
        return *(unsigned long *)dcache_read(addr);
    }
    else {               /* Let's handle the no so tricky case first: unaligned but not at a cache line boundary */
        if ((addr & dcache_line_mask & ~3) != (dcache_line_mask & ~3)) {
            #ifdef RABBITS_TRACE_EVENT
            cpu_single_env->rabbits.read_size = 4;
            #endif
            return *(unsigned long *)dcache_read(addr);
        } else { /* We are crossing a cache line boundary now, take care of it */
            uint32_t x, y, z;
            if (addr & 1) {
                z = dcache_read_b(addr + 0);
                y = dcache_read_w(addr + 1); /* half word aligned for sure, next cache line if 3 */
                x = dcache_read_b(addr + 3); /* on next cache line */
                return (x << 24) | (y << 8) | z;
            } else {
                y = dcache_read_w(addr + 0);     /* half word aligned for sure */
                x = dcache_read_w(addr + 2); /* half word aligned on next cache line */
                return (x << 16) | y;
            }
        }
    }
}

/**
 * Data cache read word
 *
 * @param addr Read address
 *
 * @return
 */
unsigned short
dcache_read_w(unsigned long addr)
{
    if((addr >= ADDR_DBG) &&
            (addr <= (ADDR_DBG+1)))
        printf("@@@@@ here W @@@@@ !!!\n");


    if ((addr & 1) == 0){/* Aligned half word access, normal case */
        #ifdef RABBITS_TRACE_EVENT
        cpu_single_env->rabbits.read_size = 2; 
        #endif
        return *(unsigned short *)dcache_read(addr);
    }
    else {/* Unaligned but not at a cache line boundary, we're safe doing the access */
        unsigned int x, y;
        y = dcache_read_b(addr);
        x = dcache_read_b(addr + 1);
        return (x << 8) | y;
    }
}

/**
 * Data cache read byte
 *
 * @param addr Read address
 *
 * @return
 */
unsigned char
dcache_read_b (unsigned long addr)
{
    if(addr == ADDR_DBG)
        printf("@@@@@ here B @@@@@ !!!\n");

    #ifdef RABBITS_TRACE_EVENT
    cpu_single_env->rabbits.read_size = 1; 
    #endif
    return *(unsigned char *) dcache_read (addr);
}

/**
 * Data cache read signed word
 *
 * @param addr Read address
 *
 * @return
 */
signed short
dcache_read_signed_w (unsigned long addr)
{
    if((addr >= ADDR_DBG) &&
            (addr <= (ADDR_DBG+1)))
        printf("@@@@@ here SW @@@@@ !!!\n");


    if ((addr & 1) == 0){/* Aligned half word access, normal case */
        #ifdef RABBITS_TRACE_EVENT
        cpu_single_env->rabbits.read_size = 2; 
        #endif
        return *(signed short *)dcache_read(addr);
    } else {
        unsigned int x, y;
        y = dcache_read_b(addr);
        x = dcache_read_b(addr + 1);
        return (x << 8) | y;
    }
}

/**
 * Data cache read signed byte
 *
 * @param addr Read address
 *
 * @return
 */
signed char
dcache_read_signed_b (unsigned long addr)
{
    if(addr >= ADDR_DBG)
        printf("@@@@@ here SB @@@@@ !!!\n");

#ifdef RABBITS_TRACE_EVENT
    cpu_single_env->rabbits.read_size = 1; 
    #endif
    return * (signed  char *) dcache_read (addr);
}

/**
 * @brief write data within the abstract (or not so abstract) cache model
 *
 * When encountering unaligned accesses that cross a cache line boundary, we recursively
 * call the function so that the next location in the cache is computed as it should.
 * Recursion is at most done one, and should occur quite seldom and it reuses cached code
 * TODO: Make sure this is Ok from a SystemC point of view
 *
 * @param addr Address to acces the data cache memory
 * @param nb Number of bytes to write (8, 16 or 32)
 * @param val value to write in cache
 *
 * @return
 */
void REGPARM
dcache_write (unsigned long addr, int nb, unsigned long val)
{
    #if HOST_LONG_BITS == 64
    if (nb == 8)
    {
        dcache_write_q (addr, val);

        return;
    }
    #endif
    
    if (!cpu_single_env || cpu_single_env->rabbits.b_use_backdoor)
    {
        fprintf (stderr, "Error in %s, env=0x%lx, backdoor=%d\n",
            __FUNCTION__, (unsigned long) cpu_single_env,
            cpu_single_env ? (int) cpu_single_env->rabbits.b_use_backdoor : 0);
        exit (1);
    }
  
    if (addr >= cpu_single_env->rabbits.ram_size)
    {
        fprintf (stderr, "Bad address 0x%lx in qemu.%s!\n",
            addr, __FUNCTION__);
        exit (1);
    }

    #ifndef IMPLEMENT_FULL_CACHES
    void   *host_addr = get_host_address(addr);

    switch (nb)
    {
    case 1:
        *((uint8_t *)host_addr)  = (uint8_t)(val & 0x000000ff);
    break;
    case 2:
        *((uint16_t *)host_addr) = (uint16_t)(val & 0x0000ffff);
    break;
    case 4:
        *((uint32_t *)host_addr) = (uint32_t)(val & 0xffffffff);
    break;
    default:
        exit (1);
    }
    #ifdef IMPLEMENT_LATE_CACHES
    g_crt_ns_misses += NS_WRITE_ACCESS;
    #endif //IMPLEMENT_LATE_CACHES
    #endif //!IMPLEMENT_FULL_CACHES

    #ifdef IMPLEMENT_FULL_CACHES
    #ifdef RABBITS_TRACE_EVENT
    int no_cycles = g_crt_no_cycles_instr;
    STOP_EXEC_PERF(); START_TLM_PERF();
    SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
    //
    if(_save_cpu_single_env->rabbits.tr_id != NULL){ // Trace Enabled
        g_crt_no_cycles_instr = 0;
        _save_crt_qemu_instance->m_systemc.systemc_trace_event(
                _save_cpu_single_env->rabbits.sc_obj); // TRACE BUFFER
    }else if(no_cycles > 0){
        g_crt_no_cycles_instr = 0;
//        crt_qemu_instance->m_counters.no_cycles += no_cycles;
        _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
            _save_cpu_single_env->rabbits.sc_obj, no_cycles);

    }
    RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
    START_EXEC_PERF(); STOP_TLM_PERF(); 
    #else //!RABBITS_TRACE_EVENT
    int no_cycles = g_crt_no_cycles_instr;
    if (no_cycles > 0) {
        g_crt_no_cycles_instr = 0;
        crt_qemu_instance->m_counters.no_cycles += no_cycles;
        STOP_EXEC_PERF(); START_TLM_PERF();
        SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
        _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
            _save_cpu_single_env->rabbits.sc_obj, no_cycles);
        RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
        START_EXEC_PERF(); STOP_TLM_PERF();
    }
        #endif
    #endif //IMPLEMENT_FULL_CACHES

    #ifdef RABBITS_GDB_ENABLED
    {
    int                 i, nb = crt_qemu_instance->m_gdb->watchpoints.nb;
    struct watch_el_t   *pwatch = crt_qemu_instance->m_gdb->watchpoints.watch;

    for (i = 0; i < nb; i++)
        if (addr >= pwatch[i].begin_address && addr < pwatch[i].end_address &&
            (pwatch[i].type == GDB_WATCHPOINT_WRITE || pwatch[i].type == GDB_WATCHPOINT_ACCESS)
            )
        {
            gdb_loop (i, 1, val);
            break;
        }
    }
    #endif //RABBITS_GDB_ENABLED

    #ifdef IMPLEMENT_FULL_CACHES

    int                 cpu = cpu_single_env->cpu_index;
    unsigned long       tag = addr >> dcache_line_bits;
    unsigned long       ofs = addr & dcache_line_mask;
    bool                skip_sysc = false;
    int                 idx, start_idx;

    start_idx = tag & (dcache_lines - 1) & ~((1 << dcache_assoc_bits) - 1);
    idx = dcache_line_present (cpu, start_idx, tag);

    #ifdef RABBITS_TRACE_EVENT
    hwe_cont* hwe_src = cpu_single_env->rabbits.tr_id;
    #ifdef RABBITS_TRACE_EVENT_CPU_REQ
    hwe_src = tr_wr_req_event(cpu, hwe_src, addr,
            TR_EVNT_PROC_DCACHE_REQ, TR_MEM_STORE);
    #endif
    #endif

    PRINTEVNT("W ",hwe_src,0,0);

    if (idx != -1 ){
        // CACHE HIT
        switch (nb)
        {
        case 1:
            *((uint8_t *)&crt_qemu_instance->m_cpu_dcache_data[cpu][idx][ofs]) =
                (uint8_t)(val & 0x000000ff);
            break;
        case 2:
            if ((ofs & 1) == 0) /* Halfword aligned write, simple case */
                *((uint16_t *)&crt_qemu_instance->m_cpu_dcache_data[cpu][idx][ofs]) =
                    (uint16_t)(val & 0x0000ffff);
            else { /* Unaligned */
                printf("case 2: Unaligned access - First access\n");
                dcache_write(addr + 0, 1, (val >> 0) & 0x000000ff);
                dcache_write(addr + 1, 1, (val >> 8) & 0x000000ff);
                skip_sysc = true;
            }
            break;
        case 4:
            if ((ofs & 3) == 0) { /* Normal, aligned word access */
                *((uint32_t *)&crt_qemu_instance->m_cpu_dcache_data[cpu][idx][ofs]) = 
                    (uint32_t)(val & 0xffffffff);
            } else {
                printf("case 4: Page boundary  First access\n");
                /* No optimization when within cache line as I believe the systemc_qemu_write_memory fails on unaligned accesses */
                if (ofs & 1) {
                    dcache_write(addr + 0, 1, (val >> 0)  & 0x000000ff); /* on current cache line */
                    dcache_write(addr + 1, 2, (val >> 8)  & 0x0000ffff); /* half word aligned for sure, maybe next cache line */
                    dcache_write(addr + 3, 1, (val >> 24) & 0x000000ff); /* on next cache line */
                } else {
                    dcache_write(addr + 0, 2, (val >> 0)  & 0x0000ffff); /* half word aligned, same cache line */
                    dcache_write(addr + 2, 2, (val >> 16) & 0x0000ffff); /* half word aligned, maybe next cache line */
                }
                skip_sysc = true;
            }
            break;
        default:
            printf("QEMU, function %s, invalid number of bytes %d\n", __func__, nb);
            exit(1);
        }

        if (!skip_sysc) { /* I am sure the address is aligned with the type now */
#ifdef CACHE_IS_WT
            #ifdef RABBITS_TRACE_EVENT
            hwe_src = tr_wr_str_event(cpu, hwe_src, addr,
                                      TR_EVNT_DCACHE_WRITE_HIT, nb);
            #endif
            STOP_EXEC_PERF(); START_TLM_PERF();
            SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
            #ifdef RABBITS_TRACE_EVENT
            _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
                _save_cpu_single_env->rabbits.sc_obj, addr, val, nb, 0,hwe_src);
            #else
            _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
                _save_cpu_single_env->rabbits.sc_obj, addr, val, nb, 0);
            #endif
            RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
            START_EXEC_PERF(); STOP_TLM_PERF(); 
#else
            if(cpu_single_env->rabbits.excl){
                dcache_invalidate(addr);
                //printf("WRT EX  : 0x%08lx val %ld\n",addr,val);
                //crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid = 0;

                if(addr == ADDR_DBG){
                    printf("WEXCL [  0x%08x   size %05d  ]\tby cpu %d valid %d dirty %d pc 0x%08x inst 0x%08x\n",(uint32_t) (addr),(uint32_t)nb,cpu,
                            crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid, crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty,
                            cpu_single_env->rabbits.tr_id->inst.body.pc,
                            cpu_single_env->rabbits.tr_id->inst.body.instr);
                }
//                crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty = 1;
                #ifdef RABBITS_TRACE_EVENT
                hwe_src = tr_wr_str_event(cpu, hwe_src, addr,
                        TR_EVNT_DCACHE_WRITEBACK, nb);
                #endif
                STOP_EXEC_PERF(); START_TLM_PERF();
                SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
                #ifdef RABBITS_TRACE_EVENT


                _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
                        _save_cpu_single_env->rabbits.sc_obj, addr, val, nb, 0,hwe_src);
                #else
                _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory (
                        _save_cpu_single_env->rabbits.sc_obj, addr, val, nb, 0);
                #endif
                RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
                START_EXEC_PERF(); STOP_TLM_PERF(); 
            }else{
                hwe_src = tr_wr_ack_event(cpu, hwe_src, addr, nb,
                        TR_EVNT_DCACHE_MODIFY, TR_MEM_MODIFY);
                PRINTEVNT("WC",hwe_src,0,0);
                STOP_EXEC_PERF(); START_TLM_PERF();
                SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
                #ifdef RABBITS_TRACE_EVENT

                if(_save_cpu_single_env->rabbits.tr_id != NULL){ // Trace Enabled
                    g_crt_no_cycles_instr = 0;
                    _save_crt_qemu_instance->m_systemc.systemc_trace_event(
                            _save_cpu_single_env->rabbits.sc_obj); // TRACE BUFFER
                }
                #endif
                RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
                START_EXEC_PERF(); STOP_TLM_PERF(); 


                /* We now have a dirty copy (no cache coherency yet) */
                crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty = 1;

                if(addr == ADDR_DBG){
                    printf("MODIF [  0x%08x   size %05d  ]\tby cpu %d valid %d dirty %d pc 0x%08x inst 0x%08x\n",(uint32_t) (addr),(uint32_t)nb,cpu,
                            crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].valid, crt_qemu_instance->m_cpu_dcache_flags[cpu][idx].dirty,
                            cpu_single_env->rabbits.tr_id->inst.body.pc,
                            cpu_single_env->rabbits.tr_id->inst.body.instr);
                }

            }
            /* Dirty hack so that it works at the end of the day, but I don't understand why and this sucks ! */

            memcpy (get_host_address(addr), &val, nb);
#endif
        }
    } else { /* Need to align writes missing the cache.
                Must be careful on cache line boundary as it may hit afterwards */
#ifdef CACHE_IS_WT
        #ifdef RABBITS_TRACE_EVENT
        hwe_src = tr_wr_str_event(cpu, hwe_src, addr,
                                  TR_EVNT_DCACHE_WRITE_MISS, nb);
        #endif
        switch (nb) {
        case 1:
            STOP_EXEC_PERF(); START_TLM_PERF();
            SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
            #ifdef RABBITS_TRACE_EVENT
            _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(_save_cpu_single_env->rabbits.sc_obj, addr, val, nb, 0,hwe_src);
            #else
            _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(_save_cpu_single_env->rabbits.sc_obj, addr, val, nb, 0); 
            #endif
            RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
            START_EXEC_PERF(); STOP_TLM_PERF(); 
            break;
        case 2:
            if ((ofs & 1) == 0) { /* Halfword aligned write, simple case */
                STOP_EXEC_PERF(); START_TLM_PERF();
                SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
                #ifdef RABBITS_TRACE_EVENT
                _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(_save_cpu_single_env->rabbits.sc_obj, addr, val, nb, 0,hwe_src);
                #else
                _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(_save_cpu_single_env->rabbits.sc_obj, addr, val, nb, 0);
                #endif
                RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
                START_EXEC_PERF(); STOP_TLM_PERF(); 

            } else { /* Unaligned */
                printf("case 2 + SystemC Access\n");
                STOP_EXEC_PERF(); START_TLM_PERF();
                SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
                #ifdef RABBITS_TRACE_EVENT
                _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(
                        _save_cpu_single_env->rabbits.sc_obj, addr, val & 0x000000ff, 1, 0,hwe_src);
                #else
                _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(
                        _save_cpu_single_env->rabbits.sc_obj, addr, val & 0x000000ff, 1, 0);
                #endif
                RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
                START_EXEC_PERF(); STOP_TLM_PERF(); 
                dcache_write(addr + 1, 1, (val >> 8) & 0x000000ff);
            }
            break;
        case 4:
            if ((ofs & 3) == 0) { /* Normal, aligned word access */
                STOP_EXEC_PERF(); START_TLM_PERF();
                SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
                #ifdef RABBITS_TRACE_EVENT
                _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(_save_cpu_single_env->rabbits.sc_obj, addr, val, nb, 0,hwe_src);
                #else
                _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(_save_cpu_single_env->rabbits.sc_obj, addr, val, nb, 0);
                #endif
                RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
                START_EXEC_PERF(); STOP_TLM_PERF(); 
            } else {
                printf("case 4 + SystemC Access\n");
                /* No optimization when within cache line as I believe the systemc_qemu_write_memory fails on unaligned accesses */
                if (ofs & 1) {
                    STOP_EXEC_PERF(); START_TLM_PERF();
                    SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
                    #ifdef RABBITS_TRACE_EVENT
                    _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(_save_cpu_single_env->rabbits.sc_obj, addr, val & 0x000000ff, 1, 0,hwe_src);
                    #else
                    _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(_save_cpu_single_env->rabbits.sc_obj, addr, val & 0x000000ff, 1, 0);
                    #endif
                    RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
                    START_EXEC_PERF(); STOP_TLM_PERF(); 
                    dcache_write(addr + 1, 2, (val >> 8)  & 0x0000ffff); /* half word aligned for sure, maybe next cache line */
                    dcache_write(addr + 3, 1, (val >> 24) & 0x000000ff); /* maybe on next cache line */
                } else {
                    STOP_EXEC_PERF(); START_TLM_PERF();
                    SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
                    #ifdef RABBITS_TRACE_EVENT
                    _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(_save_cpu_single_env->rabbits.sc_obj, addr, val & 0x0000ffff, 2, 0,hwe_src);
                    #else
                    _save_crt_qemu_instance->m_systemc.systemc_qemu_write_memory(_save_cpu_single_env->rabbits.sc_obj, addr, val & 0x0000ffff, 2, 0);
                    #endif
                    RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
                    START_EXEC_PERF(); STOP_TLM_PERF(); 
                    dcache_write(addr + 2, 2, (val >> 16) & 0x0000ffff); /* half word aligned, maybe next cache line */
                }
            }
            break;
        default:
            printf("QEMU, function %s, invalid number of bytes %d\n", __func__, nb);
//            STOP_EXEC_PERF();
            exit(1);
        }
#else /* CACHE_IS_WB */
        /* Assuming write-back is write allocate for now */
        /* Fetch the line from memory, doing whatever eviction is necessary */
        if( cpu_single_env->rabbits.tr_id != NULL ){
            cpu_single_env->rabbits.tr_id->inst.body.str = 1;
            cpu_single_env->rabbits.read_size = 0;
        }
        (void)dcache_read(addr & ~DCACHE_LINE_MASK);

        /* And now update the fetched line */
        PRINTEVNT("M ",hwe_src,0,0);
        dcache_write(addr, nb, val);
#endif
    }

    #else //!IMPLEMENT_FULL_CACHES
    qemu_invalidate_address (crt_qemu_instance, addr, cpu_single_env->cpu_index);
    #endif //IMPLEMENT_FULL_CACHES

//    STOP_EXEC_PERF();
}

/**
 * @brief Data cache write long long
 *
 * The another sizes used as DEFINE functions
 *
 * @param addr Accessed Address
 * @param val Write Value
 *
 * @return void
 */
void
dcache_write_q (unsigned long addr, unsigned long long val)
{
    dcache_write(addr + 0, 4, (unsigned long)(val & 0xffffffff));
    crt_qemu_instance->m_counters.no_mem_write--;
    dcache_write(addr + 4, 4, (unsigned long)(val >> 32));
}

/**
 * Instruction cache access
 * @param addr ...
 *
 * @return void
 */
void icache_access (target_ulong addr)
{
    int             cpu = cpu_single_env->cpu_index;
    unsigned long   tag = addr >> icache_line_bits;
    int             idx, start_idx;

    #ifdef RABBITS_TRACE_EVENT
    cpu_single_env->rabbits.tr_id = NULL;
    #endif

    start_idx = tag & (icache_lines - 1) & ~((1 << icache_assoc_bits) - 1);
    idx = icache_line_present (cpu, start_idx, tag);

    if (idx == -1)
    {
        crt_qemu_instance->m_counters.no_icache_miss++;

        idx = icache_line_replace (cpu, start_idx);
        crt_qemu_instance->m_cpu_icache_tag[cpu][idx] = tag;
        crt_qemu_instance->m_cpu_icache_flags[cpu][idx].valid = 1;

        #ifdef IMPLEMENT_FULL_CACHES
        int no_cycles = g_crt_no_cycles_instr;
        STOP_EXEC_PERF(); START_TLM_PERF();
        SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
        if (no_cycles > 0)
        {
            g_crt_no_cycles_instr = 0;
            _save_crt_qemu_instance->m_counters.no_cycles += no_cycles;
            _save_crt_qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
                _save_cpu_single_env->rabbits.sc_obj, no_cycles);
        }
        #ifdef RABBITS_TRACE_EVENT
        (void)_save_crt_qemu_instance->m_systemc.systemc_qemu_read_memory (
            _save_cpu_single_env->rabbits.sc_obj,
            addr & ~icache_line_mask, 1 << icache_line_bits, 0,0);
        #else
        (void)_save_crt_qemu_instance->m_systemc.systemc_qemu_read_memory (
            _save_cpu_single_env->rabbits.sc_obj,
            addr & ~ICACHE_LINE_MASK, ICACHE_LINE_BYTES, 0);
        #endif
        RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
        START_EXEC_PERF(); STOP_TLM_PERF(); 
        #else //cache late configuration
        g_crt_ns_misses += NS_ICACHE_MISS;
        #endif
    }
}

/**
 * Instruction cache access n times
 *
 * @param addr Initial address to acess memory cache
 * @param n    Number of cache instructions access
 *
 * @return void
 */
//void icache_access_n (target_ulong addr, int32_t n, DisasContext *s)
void icache_access_n (target_ulong addr, int32_t n)
{
   int i;
   for (i = 0; i < n; i++)
       icache_access (addr + i * 4);
}

#ifdef RABBITS_TRACE_EVENT
/**
 * @brief Cache access function for first translation block instruction or for
 * first cache line access
 *
 * Instruction cache access with trace capabilities for start of Basic
 * Translation Block or for a first cache line access. If a cache miss occurs
 * the synchronization QEMU-SystemC will be executed. Some trace events will be
 * produced (TR_EVNT_PROC_INST and TR_EVNT_PROC_ICACHE_REQ) and may another one
 * may be produced (TR_EVNT_ICACHE_REQ, TR_EVNT_ICACHE_ACK)
 *
 * @param addr Instruction Program Counter
 * @param ins ARM Instruction
 * @param type ARM instruction type ( JUMP, NORMAL OPERATION, ... )
 * @return void
 */
void icache_tr_tb_access(target_ulong  addr, target_ulong insn, target_ulong type)
{

    int             cpu = cpu_single_env->cpu_index;
    unsigned long   tag  = addr  >> icache_line_bits;
    int             idx, start_idx;
    hwe_cont*       hwe_src;
    start_idx = tag & (icache_lines - 1) & ~((1 << icache_assoc_bits) - 1);
    idx = icache_line_present (cpu, start_idx, tag);

    // Create a New Event Instruction
    hwe_src = tr_wr_inst_event(cpu, addr, insn, type, 0);

    cpu_single_env->rabbits.tr_id = hwe_src;

    // Create a Cache Request !!!
#ifdef RABBITS_TRACE_EVENT_CPU_REQ
    hwe_src = tr_wr_req_event(cpu, hwe_src, addr, TR_EVNT_PROC_ICACHE_REQ,
            TR_MEM_PREF);
#endif

    if (idx == -1) // Instruction Cache MISS
    {
        hwe_src = tr_wr_req_event(cpu, hwe_src, addr & ~icache_line_mask, TR_EVNT_ICACHE_REQ,
                                 TR_MEM_PREF);

        crt_qemu_instance->m_counters.no_icache_miss++;

        idx = icache_line_replace (cpu, start_idx);
        // TODO: Include event cache replace to instruction cache also

        crt_qemu_instance->m_cpu_icache_tag[cpu][idx] = tag;
        #ifdef IMPLEMENT_FULL_CACHES
        STOP_EXEC_PERF (); START_TLM_PERF();
        SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
        #ifdef RABBITS_TRACE_EVENT
        g_crt_no_cycles_instr = 0;
        _save_crt_qemu_instance->m_systemc.systemc_trace_event(
             _save_cpu_single_env->rabbits.sc_obj);
        #endif
        (void)_save_crt_qemu_instance->m_systemc.systemc_qemu_read_memory (
                    _save_cpu_single_env->rabbits.sc_obj,
                    addr & ~icache_line_mask, 1 << icache_line_bits, 0, hwe_src);
        RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
        START_EXEC_PERF(); STOP_TLM_PERF(); 
        #else //cache late configuration
        g_crt_ns_misses += NS_ICACHE_MISS;
        #endif

    }else{ // Instruction Cache HIT
#ifdef RABBITS_TRACE_EVENT_CACHE
        tr_wr_ack_event(cpu, hwe_src, addr, 4, TR_EVNT_ICACHE_ACK, TR_MEM_PREF);
#endif
    }

    // XXX: Ideal place to put the Write instruction, because the instructions is
    // ready to be read just after instruction cache read    

}
/**
 * @brief Cache access function for instructions on translation block
 *
 * Instruction cache access with trace capabilities specific for instructions
 * inside the Translation block. Three trace events will be produced
 * TR_EVNT_PROC_INST, TR_EVNT_PROC_ICACHE_REQ and TR_EVNT_ICACHE_ACK
 *
 * @param addr Instruction address
 * @param insn Instruction traced
 * @param type Type of instruction ( JUMP, NORMAL OPERATION, ... )
 *
 * @return void
 */
void icache_tr_inst_access(target_ulong  addr, target_ulong insn, target_ulong type)
{
    int         cpu  = cpu_single_env->cpu_index ;
    hwe_cont*   hwe_src;

    hwe_src = tr_wr_inst_event(cpu, addr, insn, type, 0);
    cpu_single_env->rabbits.tr_id = hwe_src;

#ifdef RABBITS_TRACE_EVENT_CPU_REQ
    hwe_src = tr_wr_req_event(cpu, hwe_src, addr, TR_EVNT_PROC_ICACHE_REQ,
                TR_MEM_PREF);
#endif
#ifdef RABBITS_TRACE_EVENT_CACHE
    tr_wr_ack_event(cpu, hwe_src, addr, 4, TR_EVNT_ICACHE_ACK, TR_MEM_PREF); // TODO: Verify the number o requested bytes
#endif

}

/**
 * @brief Cache access special for jump instructions
 *
 * The jump instructions needs 2 more cache accesses. The behavior is similar
 * to a normal instruction cache access.
 *
 * @param addr Access Address
 * @param type Type of Instruction
 *
 * @return void
 */
void icache_tr_jmp_access(target_ulong  addr, target_ulong dest, target_ulong type)
{
    int             cpu = cpu_single_env->cpu_index;
    unsigned long   tag  = addr  >> icache_line_bits;
    int             idx, start_idx;
    int             i = 0;
    hwe_cont*       hwe_inst_src, *hwe_src;

    hwe_inst_src = cpu_single_env->rabbits.tr_id ;

    do{
        tag = addr >> icache_line_bits;

        start_idx = tag & (icache_lines - 1) & ~((1 << icache_assoc_bits) - 1);
        idx = icache_line_present (cpu, start_idx, tag);
#ifdef RABBITS_TRACE_EVENT_CPU_REQ
        hwe_src = tr_wr_req_event(cpu, hwe_inst_src, addr, TR_EVNT_PROC_ICACHE_REQ,
                                 TR_MEM_PREF);
#else 
        hwe_src = hwe_inst_src;
#endif
        if (idx == -1)
        {
            hwe_src = tr_wr_req_event(cpu, hwe_src, addr & ~icache_line_mask, TR_EVNT_ICACHE_REQ,
                                     TR_MEM_PREF);

            crt_qemu_instance->m_counters.no_icache_miss++;

            idx = icache_line_replace (cpu, start_idx);
            // TODO: Include event cache replace to instruction cache also
            crt_qemu_instance->m_cpu_icache_tag[cpu][idx] = tag;

            #ifdef IMPLEMENT_FULL_CACHES
            STOP_EXEC_PERF(); START_TLM_PERF();
            SAVE_ENV_BEFORE_CONSUME_SYSTEMC ();
            #ifdef RABBITS_TRACE_EVENT
                g_crt_no_cycles_instr = 0;
                _save_crt_qemu_instance->m_systemc.systemc_trace_event(
                       _save_cpu_single_env->rabbits.sc_obj);
            #endif
            (void) _save_crt_qemu_instance->m_systemc.systemc_qemu_read_memory (
                        _save_cpu_single_env->rabbits.sc_obj,
                        addr & ~icache_line_mask, 1 << icache_line_bits, 0, hwe_src);
            RESTORE_ENV_AFTER_CONSUME_SYSTEMC ();
            START_EXEC_PERF(); STOP_TLM_PERF(); 
            #else //cache late configuration
            g_crt_ns_misses += NS_ICACHE_MISS;            
            #endif
        }else{
#ifdef RABBITS_TRACE_EVENT_CACHE
            tr_wr_ack_event(cpu, hwe_src, addr, 4, TR_EVNT_ICACHE_ACK, TR_MEM_PREF);
#endif
        }
        addr += 4; // Increment the address
        i++;
    }while(i < 2 ); // Do this twice

}
#endif /* RABBITS_TRACE_EVENT */

/**
 * @brief Used for mark instruction exclusive during translate phase
 *        This function is used on: gen_load_exclusive
 *
 * @return void
 */
void helper_mark_exclusive (void)
{
    target_ulong    physaddr = get_phys_addr_gdb (cpu_single_env->exclusive_addr);

   // printf("LDR SET EX  : [%d] 0x%08x\n",cpu_single_env->cpu_index, physaddr);

    crt_qemu_instance->m_systemc.memory_mark_exclusive (
         crt_qemu_instance->m_systemc.subsystem,
		 cpu_single_env->cpu_index, physaddr);

#ifdef RABBITS_TRACE_EVENT
        hwe_cont* hwe_excl = cpu_single_env->rabbits.tr_id;
        if(hwe_excl != NULL) hwe_excl->inst.body.excl = 1;
#endif

}

/**
 * Helper clrex for ARM instruction clear exclusive - clear exclusive
 * This function is used in two translated function:
 * gen_clrex
 * gen_store_exclusive
 *
 * @return void
 */
void helper_clrex (void)
{
    target_ulong    physaddr = get_phys_addr_gdb (cpu_single_env->exclusive_addr);

  //  printf("STR CLR EX 1: [%d] 0x%08x\n",cpu_single_env->cpu_index, physaddr);
    if (cpu_single_env->exclusive_addr == -1)
        return;
  //  printf("STR CLR EX 2: [%d] 0x%08x\n",cpu_single_env->cpu_index, physaddr);
    cpu_single_env->rabbits.excl = 0;

    //target_ulong    physaddr = get_phys_addr_gdb (cpu_single_env->exclusive_addr);
 
    crt_qemu_instance->m_systemc.memory_clear_exclusive (
		 crt_qemu_instance->m_systemc.subsystem,
		 cpu_single_env->cpu_index, physaddr);
}

/**
 * Strex for ARM instruction store exclusive
 ** This function is used in
 ** gen_store_exclusive
 * @return void
 */

typedef struct exclusive_addr{
    uint32_t count;
    uint32_t addr[10*1024*1024];
}exclusive_addr_t;

int32_t helper_test_exclusive (void)
{
    target_ulong    physaddr = get_phys_addr_gdb (cpu_single_env->exclusive_addr);
    int32_t ret = 0 /*, i*/;
  
/*  static exclusive_addr_t  exclusive_addr= {.count = 0};

  for(i = 0; i < exclusive_addr.count ; i++){
        if(physaddr ==  exclusive_addr.addr[i])
            break;
    }
    if(i == exclusive_addr.count){
       exclusive_addr.addr[exclusive_addr.count] = physaddr;
       printf("%x\n", exclusive_addr.addr[exclusive_addr.count]);
       exclusive_addr.count ++;
    }
*/

   ret = !crt_qemu_instance->m_systemc.memory_test_exclusive (
		 crt_qemu_instance->m_systemc.subsystem,
		 cpu_single_env->cpu_index, physaddr);

#ifdef RABBITS_TRACE_EVENT
   hwe_cont* hwe_excl = cpu_single_env->rabbits.tr_id;
   if(hwe_excl != NULL) hwe_excl->inst.body.excl = 1;
#endif

  // printf("TST EX  : [%d] 0x%08x ",cpu_single_env->cpu_index, physaddr);

   if(ret){
       cpu_single_env->rabbits.excl = 1;
       //printf("SUCCESS\n");
   }else{
       cpu_single_env->rabbits.excl = 0;
       printf("FAIL\n");
   }

   return ret; 
}
#endif //RABBITS_IMPLEMENT_CACHES
/**
 * Such function does not have access to QEMU variables directly
 */
#ifdef RABBITS_TRACE_EVENT
void
qemu_invalidate_address (qemu_instance *instance, uint32_t addr, int src_idx,
                         tr_event_grp_t type,  hwe_cont* hwe_src, uint64_t timestamp)
#else
void
qemu_invalidate_address (qemu_instance *instance, uint32_t addr, int src_idx)
#endif
{
    #ifdef RABBITS_IMPLEMENT_CACHES
    unsigned long           dtag = addr >> dcache_line_bits;
    int                     didx, dstart_idx = dtag & (dcache_lines - 1) &
                                ~((1 << dcache_assoc_bits) - 1);
    unsigned long           itag = addr >> icache_line_bits;
    int                     iidx, istart_idx = itag & (icache_lines - 1) &
                                ~((1 << icache_assoc_bits) - 1);
    qemu_instance           *old_instance = crt_qemu_instance;
    int                     i;
    crt_qemu_instance = instance;
    for (i = 0; i < instance->m_NOCPUs; i++)
    {
        if (i != src_idx && (didx = dcache_line_present (i, dstart_idx, dtag)) != -1){
            #ifdef RABBITS_TRACE_EVENT
            if(hwe_src != NULL){
                tr_wr_invalidate_event(i, addr & ~dcache_line_mask, type,hwe_src, timestamp);
            }
            #endif
            instance->m_cpu_dcache_tag  [i][didx] = (unsigned long) -1;
            instance->m_cpu_dcache_flags[i][didx].valid = (unsigned long) 0;
        }

        if ((iidx = icache_line_present (i, istart_idx, itag)) != -1){
            #ifdef RABBITS_TRACE_EVENT
            if(hwe_src != NULL){
                switch(type){
                    case TR_EVNT_DCACHE_INV:
                        tr_wr_invalidate_event(i, addr & ~icache_line_mask,
                                TR_EVNT_ICACHE_INV, hwe_src, timestamp);
                        break;
                    case TR_EVNT_DCACHE_SW_INV:
                        tr_wr_invalidate_event(i, addr & ~icache_line_mask,
                                TR_EVNT_ICACHE_SW_INV, hwe_src, timestamp);
                        break;
                    default:
                        tr_wr_invalidate_event(i, addr & ~icache_line_mask,
                                type, hwe_src, timestamp);
                        break;
                }
            }
            #endif
            instance->m_cpu_icache_tag  [i][iidx] = (unsigned long) -1;
            instance->m_cpu_icache_flags[i][iidx].valid = (unsigned long) 0;
        }
    }
    crt_qemu_instance = old_instance;
    #endif
}

#ifdef RABBITS_GDB_ENABLED
void gdb_verify (target_ulong addr)
{
    //update the un-updated registers
    cpu_single_env->rabbits.gdb_pc = addr;

    if (cpu_single_env->rabbits.sw_single_step > 0)
    {
        cpu_single_env->rabbits.sw_single_step--;
        if (cpu_single_env->rabbits.sw_single_step == 0)
        {
            cpu_single_env->exception_index = EXCP_BKPT;
            cpu_single_env->regs[15] = cpu_single_env->rabbits.gdb_pc;
            cpu_loop_exit (cpu_single_env);
            return;
        }
    }

    crt_qemu_instance->m_counters.no_instructions ++;

    if (!gdb_condition (addr))
        return;

    gdb_loop (-1, 0, 0);
}

void restore_single_step (void)
{
    if (cpu_single_env->rabbits.sw_single_step < 0)
        cpu_single_env->rabbits.sw_single_step =- 
            cpu_single_env->rabbits.sw_single_step;
}

#endif //RABBITS_GDB_ENABLED

#ifdef RABBITS_LOG_INFO
void log_pc (target_ulong addr)
{
    if (cpu_single_env->cpu_index != 0 || crt_qemu_instance->m_log_cnt_instr++ > 100000)
        return;
  
    if (crt_qemu_instance->m_fim == NULL)
    {
        if (NULL == (crt_qemu_instance->m_fim = fopen ("qemu_fim.lst", "w")))
        {
            fprintf (stderr, "Cannot open the log file for output.\n");
            exit (1);
        }
    }

    fprintf (crt_qemu_instance->m_fim, "pc= %x\tcpu= %d\n",
        (unsigned int) addr, cpu_single_env->cpu_index);

    fflush (crt_qemu_instance->m_fim);
}
#endif //RABBITS_LOG_INFO
