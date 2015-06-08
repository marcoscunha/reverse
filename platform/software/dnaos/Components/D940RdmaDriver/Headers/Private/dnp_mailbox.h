/*************************************************************************
 * Copyright (C) 2010 TIMA Laboratory                                    *
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
#ifndef __DNP_MAILBOX_H
#define __DNP_MAILBOX_H

#include <Private/dnp_rdma_pkt.h>
#include <Private/dnp_event.h>

// This parameter indicates the number of input or output 
// mailboxes which are dedicated to storing the DNP CQ events
#define DNP_MAILBOX_SIZE 1024
 
typedef enum
{
    MAILBOX_IN = 0,
    MAILBOX_OUT = 1
} dnp_mailbox_direction_t;

typedef struct mailboxS
{
    uint32_t status; //number of occupied slots
    uint32_t nr;     // index of next mail to read
    uint32_t nw;     // index of next mail to write
    dnp_event_t mail[DNP_MAILBOX_SIZE];
} dnp_mailbox_t;


/** mailbox_allocate
 *      Description: allocates the given mailbox in both directions
 */
void dnp_mailbox_allocate(uint32_t channel_id);

/** mailbox_push_mail
 *      Description: pushes a mail into the indicated  dnp eventt mailbox
 *      Verify that the mailbox is not full before....
 */
void dnp_mailbox_push_mail(uint32_t channel_id, dnp_event_t *event, mailbox_direction_t dir );



/** mailbox_pop_mail
 *      Description: pops a mail from the indicated  dnp event mailbox
 *      Verify that the mailbox is not empty before....
 */
void dnp_mailbox_pop_mail(uint32_t channel_id, dnp_event_t *event, mailbox_direction_t dir );

/** mailbox_is_full
 *      Description: returns true if the indicated mailbox for the given direction is full, false else
 */
int8_t dnp_mailbox_is_full(uint32_t channel_id, mailbox_direction_t dir );

/** mailbox_is_empty
 *      Description: returns true if the indicated mailbox for the given direction is empty, false else
 */
int8_t dnp_mailbox_is_empty(uint32_t channel_id, mailbox_direction_t dir );

#endif
