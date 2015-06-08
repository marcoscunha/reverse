#ifndef _TRACE_POWER_
#define _TRACE_POWER_

#ifdef RABBITS_TRACE_EVENT

#include "../target-arm/cpu.h"
#include "../../../components/trace_port/trace_port.h"

#define TR_EVNT_BUF_SIZE 65536 /** Must be power of 2 */

#define TR_BUF_READ  0
#define TR_BUF_WRITE 1

uint32_t             tr_init_trace(trace_port_t** trace_port);
__inline__ hwe_cont* tr_wr_inst_event(uint32_t cpu, target_ulong pc,
                                      target_ulong insn,
                                      tr_inst_t inst_grp,
                                      target_ulong dest);

__inline__ hwe_cont* tr_wr_req_event(uint32_t cpu, hwe_cont* hwe_src, uint32_t addr,
                                    tr_event_grp_t type,
                                    tr_mem_t operation);
__inline__ hwe_cont* tr_wr_str_event(uint32_t cpu, hwe_cont* hwe_src, uint32_t addr,
                                    tr_event_grp_t type, unsigned int width);
__inline__ hwe_cont* tr_wr_ack_event(uint32_t cpu, hwe_cont* hwe_src, uint32_t addr, uint32_t width,
                                     tr_inst_t type, hwe_mem_t access);
__inline__ hwe_cont* tr_wr_invalidate_event(uint32_t cpu, uint32_t addr,
                                    tr_event_grp_t type, hwe_cont* hwe_ssrc,
                                    hwe_date_t timestamp);

__inline__ hwe_cont* tr_wr_commit_event(uint32_t cpu, target_ulong insn);
void                tr_commit(target_ulong pc_after, target_ulong is_jmp);

void                tr_rd_event(tr_event_t event);
void                tr_store_events(void);

__inline__ uint32_t tr_exec_analysis(hwe_cont* hwe_inst);
uint32_t            tr_test_cc(uint32_t cond);
void                tr_unaligned_access(void);

/* QUEUE OPERATIONS */
uint32_t               tr_init_queue (void);
__inline__ hwe_cont*   tr_put_event_queue (uint32_t cpu, hwe_cont* cont);
__inline__ void        tr_update_head_queue(qemu_tr_buf_t* tr_buf);
__inline__ uint8_t     tr_is_full_queue(qemu_tr_buf_t* tr_buf);
__inline__ uint8_t     tr_is_empty_queue(qemu_tr_buf_t* tr_buf);

#endif /* RABBITS_TRACE_EVENT */

#endif

