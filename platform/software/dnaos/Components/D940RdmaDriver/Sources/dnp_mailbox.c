/*************************************************************************
 * dnp_mailbox.c : mailbox to store an event                             *
 * Copyright (C) 2008 TIMA Laboratory                                    *
 * Author: Alexandre CHAGOYA-GARZON
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/
#include <dnp/dnp.h>
#include <stdlib.h>
#include <string.h>

// global declaration of the mailboxes, one for each LUTs, one for
// each direction
dnp_mailbox_t *dnp_event_mailboxes[2*DNP_RDMA_MAILBOX_NENTRIES];

/** mailbox_allocate
 *      Description: allocates space for the mailbox in both direction
 */
void dnp_mailbox_allocate(uint32_t channel_id)
{
    mailbox_t *mb_in, *mb_out;
    mb_in =  (mailbox_t *)kernel_malloc(sizeof(mailbox_t),1); 
    mb_in -> status = 0;
    mb_in -> nr = mb_in ->nw = 0;

    mb_out =  (mailbox_t *)kernel_malloc(sizeof(mailbox_t),1); 
    mb_out -> nr = mb_out ->nw = 0;
    mb_out ->status = 0;

    dnp_event_mailboxes[2*channel_id] = mb_in;
    dnp_event_mailboxes[2*channel_id+1] = mb_out;
}

/** mailbox_push_mail
 *      Description: pushes a mail into the indicated  dnp eventt mailbox
 *      Verify that the mailbox is not full before....
 */
void dnp_mailbox_push_mail(uint32_t channel_id, dnp_event_t *event, mailbox_direction_t dir )
{
    dnp_mailbox_t *mb = event_mailboxes[2*channel_id + dir];
    dnp_event_t *slot;

    if (channel_id >= DNP_RDMA_MAILBOX_NENTRIES)
    {
        dnp_debug("[mailbox_push_mail] WARNING: channel %d has no mailbox\r\n", (int)channel_id);
        return;
    }

    if (mb -> status == DNP_MAILBOX_SIZE) return;
    slot = &mb -> mail[mb -> nw];

    *slot = *event;

    mb -> status ++;
    mb -> nw = (mb -> nw + 1) %  DNP_MAILBOX_SIZE;

}



/** mailbox_pop_mail
 *      Description: pops a mail from the indicated  dnp event mailbox
 *      Verify that the mailbox is not empty before....
 */
void dnp_mailbox_pop_mail(uint32_t mailbox_id, dnp_event_t *event, mailbox_direction_t dir )
{
    if (mailbox_id >= DNP_RDMA_MAILBOX_NENTRIES)
    {   
        dnp_debug("[mailbox_pop_mail] WARNING: channel %d has no mailbox\r\n", (int)mailbox_id);
        return;
    }
    dnp_mailbox_t *mb = dnp_event_mailboxes[2*mailbox_id + dir];
    dnp_event_t *slot;
    if (!mb -> status) return;
    slot = &mb -> mail[mb -> nr];
    
    *event = *slot;

    mb -> status --;
    mb -> nr = (mb -> nr + 1) %  DNP_MAILBOX_SIZE;

}

/** mailbox_is_full
 *      Description: returns true if the indicated mailbox for the given direction is full, false else
 */
int8_t dnp_mailbox_is_full(uint32_t mailbox_id, mailbox_direction_t dir )
{
    if (mailbox_id >= DNP_RDMA_MAILBOX_NENTRIES)
    {
        dnp_debug("[mailbox_is_full] WARNING: Mailbox %d doesn't exist!!\r\n", (int)mailbox_id);
        return false;
    }
    return (dnp_event_mailboxes[2*mailbox_id + dir] -> status == DNP_MAILBOX_SIZE );
}

/** mailbox_is_empty
 *      Description: returns true if the indicated mailbox for the given direction is empty, false else
 */
int8_t dnp_mailbox_is_empty(uint32_t mailbox_id, mailbox_direction_t dir )
{
    if (mailbox_id >= DNP_RDMA_MAILBOX_NENTRIES)
    {
        dnp_debug("[mailbox_is_empty] WARNING: channel %d doesn't exist\r\n", (int)mailbox_id);
        return true;
    }
    return (dnp_event_mailboxes[2*mailbox_id + dir] -> status == 0);
}


