#include <interconnect_master.h>
#include <generic_subsystem.h>

#if 0
#define DEBUG_GENERIC_SUBSYSTEM
#endif

#ifdef DEBUG_GENERIC_SUBSYSTEM
#define DPRINTF(fmt, args...)                               \
    do{ printf("generic_subsystem: " fmt , ##args); }while(0)
#define DCOUT if(1) cout
#else
#define DPRINTF(fmt, args...) do {} while(0)
#define DCOUT if(0) cout
#endif

#define EPRINTF(fmt, args...)                               \
    do{ fprintf(stderr, "generic_subsystem: " fmt, ##args); }while(0)


generic_subsystem::generic_subsystem(sc_module_name name, int max_masters,
                                     int max_slaves):
sc_module(name)
{
    m_masters  = new master_device *[max_masters];
    m_nmasters = 0;
    m_slaves   = new slave_device *[max_slaves];
    m_nslaves  = 0;
    
}

generic_subsystem::~generic_subsystem()
{
    DPRINTF("Generic subsystem destructor called\n");
    delete [] m_masters;
    delete [] m_slaves;
    if(m_onoc)
        delete m_onoc;
}

void
generic_subsystem::push_slave(slave_device *sl)
{
    sl->set_node_id(m_nslaves);
    m_slaves[m_nslaves++] = sl;
}


void
generic_subsystem::push_master(master_device *mast)
{
    mast->set_node_id(m_nmasters);
    m_masters[m_nmasters++] = mast;
}

void
generic_subsystem::connect_devices(interconnect_address_map_t *addr_map)
{
    int        i;
    static int n;
    const int  l = sizeof("interconnect")+4;
    char      *s = (char *)malloc(l);

    snprintf(s, l, "interconnect%d", n++);

    m_onoc = new interconnect(s, m_nmasters, m_nslaves);

    // connect slaves
    for (i = 0; i < m_nslaves; i++)
        m_onoc->connect_slave_64(i, m_slaves[i]);

    // connect masters
    for (i = 0; i < m_nmasters; i++)
        m_onoc->connect_master_64 (i, m_masters[i], addr_map);

}

slave_device *
generic_subsystem::get_slave_from_addr(uint32_t mast_id, uint32_t addr,
                                       uint32_t *mem_base_addr){

    int status;
    int slave_id;
    uint32_t base_addr = 0;
    slave_id = m_onoc->get_master(mast_id)->get_slave_id_for_mem_addr(addr);

    status = m_onoc->get_master(mast_id)->get_linear_address(slave_id, 0,
                                                             base_addr);

    if(!status) return NULL;
    
    *mem_base_addr = base_addr;

    return m_slaves[slave_id];
}

unsigned long
generic_subsystem::get_mem_addr(qemu_cpu_wrapper_t *qw, uint32_t addr)
{
    uint32_t mast_id = qw->get_node_id();
    int slave_id = m_onoc->get_master(mast_id)->get_slave_id_for_mem_addr(addr);

    if (slave_id == -1)
        return (unsigned long)dummy_for_invalid_address;
    return (unsigned long)(m_slaves[slave_id]->get_mem() + addr);

}
#ifdef TRACE_EVENT_ENABLED
void generic_subsystem::invalidate_address(uint32_t addr, int slave_id,
                                           uint32_t offset_slave, int src_id,
                                           hwe_cont* hwe)
#else
void generic_subsystem::invalidate_address(uint32_t addr, int slave_id,
                                           uint32_t offset_slave, int src_id)
#endif
{

}

void generic_subsystem::memory_mark_exclusive (int cpu, uint32_t addr)
{

}

int  generic_subsystem::memory_test_exclusive(int cpu, uint32_t addr)
{
    return 0;
}

void generic_subsystem::memory_clear_exclusive(int cpu, uint32_t addr)
{

}

extern "C"
{

#ifdef TRACE_EVENT_ENABLED
void
invalidate_address (generic_subsystem_t * sub, uint32_t addr, int slave_id,
                    uint32_t offset_slave, int src_id, hwe_cont* hwe)
{
    sub->invalidate_address(addr, slave_id, offset_slave, src_id, hwe);
}
#else
void
invalidate_address (generic_subsystem_t * sub, uint32_t addr, int slave_id,
                    uint32_t offset_slave, int src_id)
{
    sub->invalidate_address(addr, slave_id, offset_slave, src_id);
}
#endif

void
memory_mark_exclusive (generic_subsystem_t *sub, int cpu, uint32_t addr)
{
    sub->memory_mark_exclusive(cpu, addr);
}

int
memory_test_exclusive (generic_subsystem_t *sub, int cpu, uint32_t addr)
{
    return sub->memory_test_exclusive(cpu, addr);
}

void
memory_clear_exclusive (generic_subsystem_t *sub, int cpu, uint32_t addr)
{
    return sub->memory_clear_exclusive(cpu, addr);
}

unsigned long
systemc_get_mem_addr(qemu_cpu_wrapper_t *qw, generic_subsystem_t *sub, uint32_t addr)
{
    return sub->get_mem_addr(qw, addr);
}

} /* extern "C" */

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
