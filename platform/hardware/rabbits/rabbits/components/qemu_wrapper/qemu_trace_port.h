#ifndef _QEMU_TRACE_PORT_
#define _QEMU_TRACE_PORT_

#include <cfg.h>
#include <systemc.h>
#include <qemu_imported.h>

#include <hwetrace.h>
#include <hwetrace_api.h>
#include <events/hwe_device.h>
#include <events/hwe_common.h>

#include "../trace_port/trace_port.h"

//#define SC_TR_DEBUG

//*********************************
//   FIFO CLASS
//*********************************
class fifo{
private:
    hwe_cont** data;
    uint32_t   head;
    uint32_t   tail;
    uint32_t   size;
    bool       last_op;
public:
    fifo(void);
    ~fifo(void);
    hwe_cont* get(void);
    bool put(hwe_cont* item);
    void update_tail(void);
};

//*********************************
// TRACE PORT CLASS
//*********************************
class qemu_trace_port /* : public master_device */
{
private:
    uint32_t       cpuid;
    qemu_import_t* qemu_import;
    qemu_tr_buf_t* buf;
    hwe_cont**     data;

    fifo*       held_events;

    // Variables allocated here to avoid runtime reallocation
    hwe_cont *hwe_inst;
    hwe_cont *hwe_cpu_req;
    hwe_cont *hwe_cache_req;
    hwe_cont *hwe_ack;
    hwe_cont *cont;
    hwe_cont *hwe_inval;
    //signals & events

    //other attributes

    // Pointer to parent class

    // methods
    uint32_t    create_queue(uint32_t cpu_index);
    hwe_cont*   get_event_queue(void);
    void        destroy_queue(uint32_t qid);

    bool     is_empty(void);
    void     update_tail(void);

    uint32_t commit_event(void);


public:
    SC_HAS_PROCESS (qemu_trace_port);
    qemu_trace_port (qemu_instance *qemu_inst, qemu_import_t *qi, uint32_t cpu_index,
                     cache_model_t *cache);
    ~qemu_trace_port(void);

    //qemu interface
    uint32_t  systemc_trace_event (void);
    uint16_t  get_event_cycles(uint64_t timestamp);
    bool     commit_held_events(void);
    bool     commit_req_event(hwe_cont* hwe);
};

typedef qemu_trace_port qemu_trace_port_t;

#endif
