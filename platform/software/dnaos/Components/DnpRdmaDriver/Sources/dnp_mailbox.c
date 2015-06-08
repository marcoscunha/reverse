#include <MemoryManager/MemoryManager.h>
#include <Private/Dnp.h>
#include <Private/Driver.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma:mailbox"
#include <Private/debug.h>


#if 0
#include <dnp/dnp.h>
#include <stdlib.h>
#include <string.h>

#endif

// global declaration of the mailboxes, one for each LUTs, one for
// each direction
uint32_t        dnp_mailbox_nentries;
dnp_mailbox_t **dnp_mailboxes;

void
dnp_mailbox_init(uint32_t ndev)
{

  dnp_mailbox_nentries  = ndev;
  dnp_mailboxes = kernel_malloc(2*ndev*sizeof(dnp_mailbox_t *), true);

  return;
}



/** mailbox_allocate
 *      Description: allocates space for the mailbox in both direction
 */
status_t
dnp_mailbox_allocate(uint32_t channel_idx)
{

    dnp_mailbox_t *mb_in, *mb_out;
    status_t ret_val;

  watch (status_t)
  {
    mb_in =  (dnp_mailbox_t *)kernel_malloc(sizeof(dnp_mailbox_t),1); 
    mb_in -> status = 0;
    mb_in -> nr = mb_in ->nw = 0;
    ret_val = semaphore_create("mailbox_in", 0, &mb_in->sem);
    check (sem_error, ret_val == DNA_OK, DNA_ERROR);

    mb_out =  (dnp_mailbox_t *)kernel_malloc(sizeof(dnp_mailbox_t),1); 
    mb_out -> nr = mb_out ->nw = 0;
    mb_out ->status = 0;
    ret_val = semaphore_create("mailbox_out", 0, &mb_out->sem);
    check (sem_error, ret_val == DNA_OK, DNA_ERROR);

    /* Init all the semaphores */
    while(semaphore_acquire(mb_in->sem, 1, DNA_RELATIVE_TIMEOUT, 0) 
        == DNA_OK);
    while(semaphore_acquire(mb_out->sem, 1, DNA_RELATIVE_TIMEOUT, 0) 
        == DNA_OK);

    dnp_mailboxes[2*channel_idx] = mb_in;
    dnp_mailboxes[2*channel_idx+1] = mb_out;

    return DNA_OK;
  }
  rescue (sem_error)
  {
    EMSG("Failed: no sem initialized");
    leave;
  }

}

/** mailbox_push_mail
 *      Description: pushes a mail into the indicated  dnp eventt mailbox
 *      Verify that the mailbox is not full before....
 */
void
dnp_mailbox_push_mail(uint32_t virt_channel_id, dnp_event_t *event,
		      dnp_mailbox_direction_t dir )
{

    uint32_t channel_idx = dnp_channels_virt_to_dev[virt_channel_id];
    dnp_mailbox_t *mb = dnp_mailboxes[2*channel_idx + dir];
    dnp_event_t *slot;

    if (channel_idx >= dnp_mailbox_nentries)
    {
      DMSG("[push] WARNING: channel %u(%u) has no mailbox\r\n",
	   virt_channel_id, channel_idx);
      return;
    }

    if (mb->status == DNP_MAILBOX_SIZE){
      EMSG("[push] Dropped mail on channel %u\n", channel_idx);
      return;
    }
    slot = &mb->mail[mb->nw];

    *slot = *event;

    mb->status ++;
    mb->nw = (mb->nw + 1) %  DNP_MAILBOX_SIZE;
    semaphore_release(mb->sem, 1, DNA_NO_RESCHEDULE);

}

/** mailbox_pop_mail
 *      Description: pops a mail from the indicated  dnp event mailbox
 *      Verify that the mailbox is not empty before....
 */
dnp_status_t
dnp_mailbox_pop_mail(uint32_t virt_channel_id, dnp_event_t *event,
		     dnp_mailbox_direction_t dir, int blocking)
{
  dnp_mailbox_t *mb = NULL;
  dnp_event_t *slot;
  uint32_t channel_idx = dnp_channels_virt_to_dev[virt_channel_id];
  status_t res;
  int32_t flags = 0;

  if(channel_idx >= dnp_mailbox_nentries){   
    DMSG("[pop_] WARNING: channel %u(%u) has no mailbox\r\n",
	 virt_channel_id, channel_idx);
    return DNP_MAIL_ERROR;
  }
  mb = dnp_mailboxes[2*channel_idx + dir];
  
  if(!blocking)
    flags = DNA_RELATIVE_TIMEOUT;

  res = semaphore_acquire(mb->sem, 1, flags, 0);

  if(res != DNA_OK && !blocking){
    return DNP_NO_MAIL;
  }

  if(res != DNA_OK && blocking){
    EMSG("+++ got an error with semaphore : %x\n", res);
    return DNP_MAIL_ERROR;
  }

  if (!mb->status) return DNP_NO_MAIL;
  slot = &mb->mail[mb -> nr];
  
  *event = *slot;
  
  mb->status --;
  mb->nr = (mb -> nr + 1) %  DNP_MAILBOX_SIZE;
  
  return DNP_SUCCESS;
}

/** mailbox_is_full
 *      Description: returns true if the indicated mailbox for the given direction is full, false else
 */
int8_t dnp_mailbox_is_full(uint32_t virt_channel_id, dnp_mailbox_direction_t dir )
{

  uint32_t channel_idx = dnp_channels_virt_to_dev[virt_channel_id];
  if (channel_idx >= dnp_mailbox_nentries){
    DMSG("[is_full] WARNING: Mailbox %u(%u) doesn't exist!!\r\n",
	 virt_channel_id, channel_idx);
    return false;
  }
  return (dnp_mailboxes[2*channel_idx + dir] -> status == DNP_MAILBOX_SIZE );
}

/** mailbox_is_empty
 *      Description: returns true if the indicated mailbox for the given direction is empty, false else
 */
int8_t
dnp_mailbox_is_empty(uint32_t virt_channel_id, dnp_mailbox_direction_t dir)
{
  uint32_t channel_idx = dnp_channels_virt_to_dev[virt_channel_id];
  if(channel_idx >= dnp_mailbox_nentries){
    DMSG("[is_empty] WARNING: channel %u(%u) doesn't exist\r\n",
	 virt_channel_id, channel_idx);
    return true;
  }
  return (dnp_mailboxes[2*channel_idx + dir]->status == 0);
}

