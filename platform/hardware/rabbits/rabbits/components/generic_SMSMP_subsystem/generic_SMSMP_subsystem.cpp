#include <interconnect_master.h>

#include <generic_SMSMP_subsystem.h>
#include <generic_subsystem.h>

#if 0
#define DEBUG_GENERIC_SMSMP_SUBSYSTEM
#endif

#ifdef DEBUG_GENERIC_SUBSYSTEM
#define DPRINTF(fmt, args...)                               \
    do{ printf("generic_SMSMP_subsystem: " fmt , ##args); }while(0)
#define DCOUT if(1) cout
#else
#define DPRINTF(fmt, args...) do {} while(0)
#define DCOUT if(0) cout
#endif

#define EPRINTF(fmt, args...)                               \
    do{ fprintf(stderr, "generic_SMSMP_subsystem: " fmt, ##args); }while(0)


generic_SMSMP_subsystem::generic_SMSMP_subsystem(sc_module_name name, int max_masters,
                                                 int max_slaves):
generic_subsystem(name, max_masters, max_slaves)
{
    no_mem_exclusive = 0;
}

generic_SMSMP_subsystem::~generic_SMSMP_subsystem()
{
    DPRINTF("Generic SMSMP subsystem destructor called\n");
}

void
generic_SMSMP_subsystem::push_qemu_wrapper(qemu_wrapper *qemu, int ncpus)
{
    m_qemu = qemu;
    m_ncpus = ncpus;
}

#ifdef TRACE_EVENT_ENABLED
void
generic_SMSMP_subsystem::invalidate_address(uint32_t addr, int slave_id,
                                            uint32_t offset_slave, int src_id, hwe_cont* hwe)
#else
void
generic_SMSMP_subsystem::invalidate_address(uint32_t addr, int slave_id,
                                            uint32_t offset_slave, int src_id)
#endif
{
    int first_node_id;
    uint32_t taddr;

    first_node_id = m_qemu->m_cpus[0]->get_node_id(); 


    if( (src_id >= first_node_id) &&
        (src_id < first_node_id + m_qemu->m_ncpu) ){
#ifdef TRACE_EVENT_ENABLED
        m_qemu->invalidate_address (addr, src_id - first_node_id, hwe);
#else
        m_qemu->invalidate_address (addr, src_id - first_node_id);
#endif
    }else{
        if(m_onoc->get_master(first_node_id)->get_linear_address(slave_id,
                                                                 offset_slave, taddr)){
#ifdef TRACE_EVENT_ENABLED
            m_qemu->invalidate_address (taddr, -1, hwe);
#else
            m_qemu->invalidate_address (taddr, -1);
#endif
        }
    }
  
}

void
generic_SMSMP_subsystem::memory_mark_exclusive(int cpu, uint32_t addr)
{
    int             i;

    DPRINTF("Using generic mark_ex\n");
    addr &= 0xFFFFFFFC;

    for (i = 0; i < no_mem_exclusive; i++)
        if (addr == mem_exclusive[i].addr)
            break;
    
    if(i >= no_mem_exclusive){
        mem_exclusive[no_mem_exclusive].addr = addr;
        mem_exclusive[no_mem_exclusive].cpu = cpu;
        no_mem_exclusive++;

        if(no_mem_exclusive > m_ncpus){
            printf("Warning: number of elements in the exclusive list (%d) > cpus (%d) (list: ",
                no_mem_exclusive, m_ncpus);
            for(i = 0; i < no_mem_exclusive; i++)
                printf("%x ", mem_exclusive[i].addr);
            printf (")\n");
        }
    }
}

int
generic_SMSMP_subsystem::memory_test_exclusive(int cpu, uint32_t addr)
{
    int             i;

    addr &= 0xFFFFFFFC;

    DPRINTF("Using generic test_ex\n");

    for (i = 0; i < no_mem_exclusive; i++)
        if (addr == mem_exclusive[i].addr)
            return (cpu != mem_exclusive[i].cpu);

    return 1;
}

void
generic_SMSMP_subsystem::memory_clear_exclusive(int cpu, uint32_t addr)
{
    int             i;

    addr &= 0xFFFFFFFC;

    DPRINTF("Using generic clear_ex\n");

    for (i = 0; i < no_mem_exclusive; i++)
        if (addr == mem_exclusive[i].addr)
        {
            for (; i < no_mem_exclusive - 1; i++)
            {
                mem_exclusive[i].addr = mem_exclusive[i + 1].addr;
                mem_exclusive[i].cpu = mem_exclusive[i + 1].cpu;
            }
            
            no_mem_exclusive--;
            return;
        }

    printf("Warning in %s: cpu %d not in the exclusive list: ",
        __FUNCTION__, cpu);
    for(i = 0; i < no_mem_exclusive; i++)
        printf ("(%x, %d) ", mem_exclusive[i].addr, mem_exclusive[i].cpu);
    printf ("\n");
}

/*
 * Vim standard variables
 * vim:set sw=4 ts=4 expandtab tw=80 cindent syntax=c:
 *
 * Emacs standard variables
 * Local Variables:
 * mode: c
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
