#include <stdio.h>
#include <stdlib.h>

#include "rabbits/cfg.h"
#include "qemu-common.h"
#include "cpu.h"
#include "hw/loader.h"
#include "rabbits/qemu_systemc.h"
#include "rabbits/gdb_srv.h"
#ifdef RABBITS_TRACE_EVENT
#include "trace_power.h"
#endif

#include "../../../components/qemu_wrapper/qemu_imported.h"

#define MAX_IOPORTS     (64 * 1024) /**<  Maximum number of IO Ports*/

qemu_instance           *crt_qemu_instance = NULL;
qemu_instance           *qemu_instances[128]; /* TODO: BKS */
int                     no_qemu_instances = 0;
target_ulong            dcache_line_mask;
target_ulong            icache_line_mask;
int                     dcache_assoc_bits;
int                     icache_assoc_bits;
int                     dcache_line_bits;
int                     icache_line_bits;
int                     dcache_lines;
int                     icache_lines;
int                     dcache_line_bytes;

static void sigsegv_h (int x)
{
    printf ("SIGSEGV signal received! (%d)\n", x);
    //exit(1);
	qemu_instances[0]->m_systemc.systemc_stop();
}

static void sigabrt_h (int x)
{
    printf ("SIGABRT signal received! (%d)\n", x);
}

static void sigint_h (int x)
{
    printf("SIGINT\n");
    if (!gdb_start_debug ()){
		 //exit(2);
		 qemu_instances[0]->m_systemc.systemc_stop();
	}
}

int irq_pending (CPUState *penv)
{
    #if defined(TARGET_ARM)
        return (penv->interrupt_request & (CPU_INTERRUPT_FIQ | CPU_INTERRUPT_HARD));
    #else
        #error CPU not implemented in irq_pending
    #endif
}

/***********************************************************/
/* main execution loop */

static long
qemu_cpu_loop (CPUState *penv)
{
    crt_qemu_instance = penv->rabbits.qemu_instance;

    int             ret = cpu_exec (penv);

    unsigned long   no_cycles = g_crt_no_cycles_instr;
    g_crt_no_cycles_instr = 0;

    #ifdef IMPLEMENT_LATE_CACHES
    unsigned long   ns_misses = g_crt_ns_misses;
    if (ns_misses)
    {
        g_crt_ns_misses = 0;
        penv->rabbits.qemu_instance->m_systemc.systemc_qemu_consume_ns (ns_misses);
    }
    #endif

    if (no_cycles)
    {
#ifdef RABBITS_TRACE_EVENT
        penv->rabbits.qemu_instance->m_systemc.systemc_trace_event(
                          penv->rabbits.sc_obj);
#else
        penv->rabbits.qemu_instance->m_counters.no_cycles += no_cycles;
        penv->rabbits.qemu_instance->m_systemc.systemc_qemu_consume_instruction_cycles (
            penv->rabbits.sc_obj, no_cycles);
#endif
    }
    if ((ret == EXCP_HLT || ret == EXCP_HALTED) && irq_pending (penv))
        ret = EXCP_INTERRUPT;
	crt_qemu_instance = NULL;

  return ret;
}

static void
qemu_cpu_start(CPUState *penv, unsigned long index){


	 qemu_instance *save_instance;
	 qemu_instance *inst = penv->rabbits.qemu_instance;

	 save_instance = crt_qemu_instance;
	 crt_qemu_instance = inst;
	 
	 penv->rabbits.sc_mem_host_addr = (unsigned long)
		  inst->m_systemc.systemc_get_mem_addr(penv->rabbits.sc_obj,
		                                       inst->m_systemc.subsystem,
		                                       0);

	 if (!index) {
	    qemu_ram_alloc_from_ptr(NULL, "ram", inst->m_ram_size, 
		                       (void *)penv->rabbits.sc_mem_host_addr);
	    qemu_ram_alloc_from_ptr(NULL, "arm_smp_boot", 0x1000, 
		                       (void *)(penv->rabbits.sc_mem_host_addr + inst->m_ram_size));
     }

	 crt_qemu_instance = save_instance;

}



/***********************************************************/
/* lib entry point */
qemu_import_t qemu_imports = {
	 .qemu_add_map =            qemu_add_map,
	 .qemu_release =            qemu_release,
	 .qemu_get_set_cpu_obj =    qemu_get_set_cpu_obj,
	 .qemu_cpu_start =          qemu_cpu_start,
	 .qemu_cpu_loop =           qemu_cpu_loop,
	 .qemu_irq_update =         qemu_irq_update,
	 .qemu_get_counters =       qemu_get_counters,
	 .qemu_invalidate_address = qemu_invalidate_address,
	 .gdb_srv_start_and_wait =  gdb_srv_start_and_wait,
#ifdef RABBITS_TRACE_EVENT
	 .qemu_get_tr_buf =         qemu_get_tr_buf,
     .qemu_get_set_trace =      qemu_get_set_trace,
#endif
     .qemu_get_perf  =          qemu_get_perf,
};

#ifdef RABBITS_TRACE_EVENT
void *
glue(TARGET_BASE_ARCH_, _qemu_init) (
    int id, int ncpu, const char *cpu_model, int _ramsize,
    cache_model_t* cache_model, trace_port_t** trace_port,
    struct qemu_import_t *qi, struct systemc_import_t *systemc_fcs);

void *
glue(TARGET_BASE_ARCH_, _qemu_init) (
    int id, int ncpu, const char *cpu_model, int _ramsize,
    cache_model_t* cache_model, trace_port_t** trace_port,
    struct qemu_import_t *qi, struct systemc_import_t *systemc_fcs)
#else
void *
glue(TARGET_BASE_ARCH_, _qemu_init) (
    int id, int ncpu, const char *cpu_model, int _ramsize,
    struct qemu_import_t *qi, struct systemc_import_t *systemc_fcs);
void *
glue(TARGET_BASE_ARCH_, _qemu_init) (
    int id, int ncpu, const char *cpu_model, int _ramsize, struct qemu_import_t *qi,
	struct systemc_import_t *systemc_fcs)
#endif
{
    signal (SIGSEGV, sigsegv_h);
    signal (SIGABRT, sigabrt_h);
    signal (SIGINT, sigint_h);

    *qi = qemu_imports;

    //init current qemu simulator "object"
    crt_qemu_instance = calloc(1, sizeof(qemu_instance));
    crt_qemu_instance->m_systemc = *systemc_fcs;
    crt_qemu_instance->m_NOCPUs = ncpu;
    crt_qemu_instance->m_id = id;
    crt_qemu_instance->m_ram_size = _ramsize;
    crt_qemu_instance->m_io_mem_opaque = calloc(1, IO_MEM_NB_ENTRIES * sizeof(void *));
    crt_qemu_instance->m_io_mem_read   = calloc(1, IO_MEM_NB_ENTRIES * 4 * sizeof(CPUReadMemoryFunc *));
    crt_qemu_instance->m_io_mem_write  = calloc(1, IO_MEM_NB_ENTRIES * 4 * sizeof (CPUWriteMemoryFunc *));
    crt_qemu_instance->m_io_mem_used   = calloc(1, IO_MEM_NB_ENTRIES * sizeof (char));
    exec_c_init ();

    // Cache parameters
    #ifdef RABBITS_IMPLEMENT_CACHES
    dcache_line_bits  = DCACHE_LINE_BITS;
    dcache_lines      = DCACHE_LINES;
    dcache_line_mask  = DCACHE_LINE_MASK;
    dcache_assoc_bits = DCACHE_ASSOC_BITS;
    dcache_line_bytes = DCACHE_LINE_BYTES;

    icache_lines      = ICACHE_LINES;
    icache_line_bits  = ICACHE_LINE_BITS;
    icache_line_mask  = ICACHE_LINE_MASK;
    icache_assoc_bits = ICACHE_ASSOC_BITS;
    #endif

    qemu_instances[no_qemu_instances++] = crt_qemu_instance;

    #ifdef RABBITS_TRACE_EVENT
    tr_init_trace(trace_port);
    #endif
    /* init the dynamic translator */
    cpu_exec_init_all ();

    glue(TARGET_BASE_ARCH_,_generic_machine_init) (_ramsize, cpu_model, NULL);
    #ifdef RABBITS_IMPLEMENT_CACHES
    qemu_init_caches();
    #endif //RABBITS_IMPLEMENT_CACHES

    CPUState    *penv = (CPUState *) crt_qemu_instance->m_first_cpu;
    int         cpu_index = 0;
    crt_qemu_instance->m_envs = malloc (ncpu * sizeof (CPUState *));
    while (penv)
    {
        crt_qemu_instance->m_envs[cpu_index++] = penv;
        penv = penv->next_cpu;
    }

    srandom (time (NULL));

    //gdb server
    gdb_server_init (crt_qemu_instance);

    return crt_qemu_instance;
}

//defined in vl.c
const char      *mem_path = NULL;
int             mem_prealloc = 0;
int             singlestep = 0;
int             semihosting_enabled = 0;

//defined in osdep.c
int qemu_madvise (void *addr, size_t len, int advice)
{
    return -1;
}

//defined in cpus.c
int qemu_cpu_is_self (void *env)
{
    return 1;
}

void qemu_cpu_kick (void *env)
{
}

void qemu_init_vcpu(void *_env)
{
}

//defined in arm-semi.c
uint32_t do_arm_semihosting (CPUState *env)
{
    return 0;
}

//defined in loader.c
void *rom_ptr (target_phys_addr_t addr)
{
    return NULL;
}

#include "hw/xen.h"
void xen_ram_alloc(ram_addr_t ram_addr, ram_addr_t size)
{
}

