#ifndef _QEMU_ENCAP_
#define _QEMU_ENCAP_


struct GDBState;

typedef struct qemu_instance qemu_instance;

/*
 * Remove the relative include (in systemC world)
 */
#include "rabbits/cfg.h"
#include "rabbits/systemc_imports.h"
#include "../../../components/qemu_wrapper/qemu_imported.h"

#ifdef RABBITS_IMPLEMENT_CACHES
typedef struct {
   uint8_t valid:1;
   uint8_t dirty:1;
   uint8_t state:3;
} dline_flags;

typedef struct {
   uint8_t valid:1;
} iline_flags;
#endif

struct qemu_instance 
{
    int                     m_id;
    int                     m_NOCPUs;

    #ifdef RABBITS_IMPLEMENT_CACHES
    dline_flags             (*m_cpu_dcache_flags)[DCACHE_LINES];
    unsigned long           (*m_cpu_dcache_tag)[DCACHE_LINES];
    unsigned char           (*m_cpu_dcache_data)[DCACHE_LINES][DCACHE_LINE_BYTES];
    iline_flags             (*m_cpu_icache_flags)[ICACHE_LINES];
    unsigned long           (*m_cpu_icache_tag)[ICACHE_LINES];
    unsigned char           (*m_cpu_icache_data)[ICACHE_LINES][ICACHE_LINE_BYTES];
    #endif

    void                    **irqs_systemc;

    void                    *m_first_cpu;
    void                    **m_envs;
    void                    *m_io_mem_write;
    void                    *m_io_mem_read;
    void                    *m_io_mem_opaque;
    char                    *m_io_mem_used;
    int                     m_io_mem_watch;
    void                    *m_l1_map;
    void                    **m_l1_phys_map;
    void                    *m_ram_list;
    int                     m_ram_size;
    void                    *m_tb_phys_hash;
    void                    *m_tbs;
    int                     m_nb_tbs;
    uint8_t                 *m_code_gen_buffer;
    unsigned long           m_code_gen_buffer_max_size;
    unsigned long           m_code_gen_buffer_size;
    uint8_t                 *m_code_gen_ptr;
    int                     m_code_gen_max_blocks;
    unsigned long           m_flush_head;

    struct GDBState         *m_gdb;

    struct systemc_import_t m_systemc;
    struct qemu_counters_t  m_counters;

    //log
    FILE                    *m_fim;
    FILE                    *m_fdm;
    unsigned long           m_log_cnt_instr;
    unsigned long           m_log_cnt_data;
#ifdef RABBITS_TRACE_EVENT
    qemu_tr_buf_t          *m_tr_buf; // TODO: Include this structure inside qemu_trace_t
    struct qemu_trace_t     trace; 
#endif
    struct qemu_perf_t      perf;
};

extern qemu_instance        *crt_qemu_instance;

#endif
