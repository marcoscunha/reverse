#ifndef __DNP_EVENT_QUEUE_H__
#define __DNP_EVENT_QUEUE_H__

#include <Private/dnp_rdma_pkt.h>
#include <Private/status.h>

/*
 * Driver representation of a completion queue event from the DNP
 */

/*
 * Events
 */
typedef struct dnp_event dnp_event_t;
typedef enum dnp_rdma_event_type dnp_rdma_event_type_t;

enum dnp_rdma_event_type {
    RDMA_EVENT_TYPE_NONE,
    RDMA_EVENT_TYPE_PUT_ACK, // put acknowledgement
    RDMA_EVENT_TYPE_PUT_REQ, // incoming put
    RDMA_EVENT_TYPE_GET_REQ, // get request from target
    RDMA_EVENT_TYPE_GET_RSP, // wait get request from target
    RDMA_EVENT_TYPE_GET_ACK, // wait get request from target
    RDMA_EVENT_TYPE_END
};

struct dnp_event {
   dnp_rdma_event_type_t type;
   rdma_pkt_types_t      pkt_type;
   uint32_t len;
   uint32_t ptr;
   uint32_t channel_id;
   rdma_pkt_t *pkt;
};

/*  dnp_event_init
 *      Description: initializes  the DNP RB
 */
void dnp_event_queue_init();

/*  dnp_event_open
 *      Description: allocates a mailbox 
 */
void dnp_event_open(uint32_t channel_id);

/*  dnp_event_request
 *      Description: checks if an event type is in the mailbox, if not returns 0
 */
dnp_status_t dnp_event_request(uint32_t channel_id, uint32_t  mb_dir, dnp_event_t *event, int blocking);

/*  dnp_poll: very important function of the driver
 *      Description: reads all CQ events, put in the events mailbox
 */
void dnp_event_poll(void);

#endif /* __DNP_EVENT_QUEUE_H__ */
