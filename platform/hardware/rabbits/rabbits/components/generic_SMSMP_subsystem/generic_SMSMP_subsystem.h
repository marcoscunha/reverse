#ifndef __GENERIC_SMSMP_SUBSYSTEM_H__
#define __GENERIC_SMSMP_SUBSYSTEM_H__

#include <master_device.h>
#include <slave_device.h>
#include <interconnect.h>
#include <qemu_wrapper.h>
#include <qemu_cpu_wrapper.h>
#include <generic_subsystem.h>

extern "C" {
    typedef struct subsystem_wrapper subsystem_wrapper_t;
}

class generic_SMSMP_subsystem : public generic_subsystem {

public:
    qemu_wrapper         *m_qemu;
    int                   m_ncpus;
public:
	generic_SMSMP_subsystem(sc_module_name name, int nb_masters, int nb_slaves);
	~generic_SMSMP_subsystem();

	void push_qemu_wrapper(qemu_wrapper *qemu, int nb_cpus);

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

private:
    unsigned char dummy_for_invalid_address[256];
    struct mem_exclusive_t {uint32_t addr; int cpu;} mem_exclusive[100];
    int no_mem_exclusive;

};

#endif /* __GENERIC_SMSMP_SUBSYSTEM_H__ */

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
