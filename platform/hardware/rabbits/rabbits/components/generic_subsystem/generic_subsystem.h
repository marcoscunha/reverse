#ifndef __GENERIC_SUBSYSTEM_H__
#define __GENERIC_SUBSYSTEM_H__

class generic_subsystem;
typedef generic_subsystem generic_subsystem_t;

#include <master_device.h>
#include <slave_device.h>
#include <interconnect.h>
#include <qemu_wrapper.h>
#include <qemu_cpu_wrapper.h>

class generic_subsystem : public sc_module {

public:
    slave_device        **m_slaves;
    int                   m_nslaves;
    master_device       **m_masters;
    int                   m_nmasters;
	interconnect         *m_onoc;
public:
	generic_subsystem(sc_module_name name, int nb_masters, int nb_slaves);
	~generic_subsystem();

	void push_slave(slave_device *sl);
	void push_master(master_device *mast);
	void connect_devices(interconnect_address_map_t *addr_map);

    slave_device *get_slave_from_addr(uint32_t mast_id, uint32_t addr,
                                      uint32_t *mem_base_addr);

    virtual unsigned long get_mem_addr(qemu_cpu_wrapper_t *qw, uint32_t addr);
#ifdef TRACE_EVENT_ENABLED
    virtual void invalidate_address (uint32_t addr, int slave_id, uint32_t offset_slave,
                             int src_id, hwe_cont* hwe);
#else
    virtual void invalidate_address (uint32_t addr, int slave_id, uint32_t offset_slave,
                             int src_id);
#endif

    virtual void memory_mark_exclusive (int cpu, uint32_t addr);
    virtual int  memory_test_exclusive(int cpu, uint32_t addr);
    virtual void memory_clear_exclusive(int cpu, uint32_t addr);
    
    inline generic_subsystem *get_subsystem(void) { return this; };

private:
    unsigned char dummy_for_invalid_address[256];

};

#endif /* __GENERIC_SUBSYSTEM_H__ */

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
