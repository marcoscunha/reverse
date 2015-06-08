#ifndef __DNP_MAILBOX_H__
#define __DNP_MAILBOX_H__

#include <Private/dnp_event_queue.h>
#include <Core/Semaphore.h>
#include <Private/status.h>

#if 0
#include <Private/dnp_rdma_pkt.h>

#endif

#define DNP_MAILBOX_SIZE          64

typedef struct dnp_mailbox dnp_mailbox_t; 
typedef enum dnp_mailbox_direction dnp_mailbox_direction_t;

enum dnp_mailbox_direction {
    MAILBOX_IN = 0,
    MAILBOX_OUT = 1
};

struct dnp_mailbox {
    uint32_t status; //number of occupied slots
    uint32_t nr;     // index of next mail to read
    uint32_t nw;     // index of next mail to write
    int32_t  sem;
    dnp_event_t mail[DNP_MAILBOX_SIZE];
};


void dnp_mailbox_init(uint32_t ndev);

/** mailbox_allocate
 *      Description: allocates the given mailbox in both directions
 */
status_t dnp_mailbox_allocate(uint32_t channel_idx);

/** mailbox_pop_mail
 *      Description: pops a mail from the indicated  dnp event mailbox
 *      Verify that the mailbox is not empty before....
 */
dnp_status_t dnp_mailbox_pop_mail(uint32_t channel_id, dnp_event_t *event, 
				  dnp_mailbox_direction_t dir, int blocking);

/** mailbox_push_mail
 *      Description: pushes a mail into the indicated  dnp eventt mailbox
 *      Verify that the mailbox is not full before....
 */
void dnp_mailbox_push_mail(uint32_t channel_id, dnp_event_t *event,
						   dnp_mailbox_direction_t dir );

/** mailbox_is_full
 *      Description: returns true if the indicated mailbox for the given 
 *      direction is full, false else
 */
int8_t dnp_mailbox_is_full(uint32_t channel_id, dnp_mailbox_direction_t dir );

/** mailbox_is_empty
 *      Description: returns true if the indicated mailbox for the given
 *      direction is empty, false else
 */
int8_t dnp_mailbox_is_empty(uint32_t channel_id, dnp_mailbox_direction_t dir );


#endif /* __DNP_MAILBOX_H__ */
