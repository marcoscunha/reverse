#ifndef __DNP_H__
#define __DNP_H__


#include <Private/dnp_topology.h>
#include <Private/dnp_event_queue.h>
#include <Private/dnp_mailbox.h>

#ifndef __DNP_FIXOUILLE__
#include <Private/dnp_types.h>
#else
#include <Private/dnp_types_rev.h>
#endif

/*
 * API between the hardware dependant part of the driver and the generic one
 */


void rdma_engine_reset(void);

/*
 * Initialize hardware-dependant part of the driver
 */
void rdma_engine_init(void);

/*
 * Open a channel
 */
void rdma_engine_open(uint32_t channel_id);

/*
 * Register a streaming buffer for PUT operations
 */
int32_t rdma_engine_register_streaming_buffer(int8_t *startptr, int32_t len,
											  uint32_t magic);

/*
 * Register a regular  src or target buffer zone for a GET operation
 */
int32_t rdma_engine_register_buffer(int8_t *startptr, int32_t len,
									uint32_t magic);

/*
 * Unregister regular buffers
 */
int32_t rdma_engine_unregister_buffer(uint32_t channel_id);


/*
 * Get a rdma pkt, thhis method is non-blocking to allow the generic part
 * to yield the task
 */
int32_t rdma_engine_get_pkt(uint32_t channel_id, rdma_pkt_t **pkt, int blocking);

/*
 * Send a rdma pkt using a PUT targeting a streming buffer
 */
int32_t rdma_engine_send_pkt(uint32_t rank, uint32_t dev, rdma_pkt_t *pkt);

/*
 * Initiate a GET request to peek data from remote tile
 */
int32_t rdma_engine_get(void *dst, uint32_t dest_rank, uint32_t dest_dev,
						void *src, uint32_t nwords, uint32_t channel_id);

/*
 * Check completion of GET request
 */
uint32_t rdma_engine_test(uint32_t channel_id);


#if 0
/* APENET version */
void dnp_micro_send_cmd(dnp_micro_cmd_reg_buf_t *cmd);
#endif

#endif /* __DNP_H__ */
