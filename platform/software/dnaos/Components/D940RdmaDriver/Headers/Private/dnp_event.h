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

#ifndef __DNP_EVENT_H
#define __DNP_EVENT_H

/*   Events     */
#include <Private/dnp_rdma_pkt.h>

typedef enum {
    RDMA_EVENT_TYPE_NONE,
    RDMA_EVENT_TYPE_PUT_ACK, // put acknowledgement
    RDMA_EVENT_TYPE_PUT_REQ, // incoming put
    RDMA_EVENT_TYPE_GET_REQ, // get request from target
    RDMA_EVENT_TYPE_GET_RSP, // wait get request from target
    RDMA_EVENT_TYPE_GET_ACK, // wait get request from target
    RDMA_EVENT_TYPE_END
} dnp_rdma_event_type_t;


// Driver representation of a completion queue event from the DNP
typedef struct _dnp_event_t
{
   dnp_rdma_event_type_t type;
   rdma_pkt_types_t      pkt_type;
   uint32_t len;
   uint32_t ptr;
   uint32_t channel_id;
   rdma_pkt_t *pkt;
} dnp_event_t;


/*  dnp_event_init
 *      Description: initializes  the DNP RB
 */
void dnp_event_init();

/*  dnp_event_open
 *      Description: allocates a mailbox 
 */
void dnp_event_open(uint32_t channel_id);


/*  dnp_event_request
 *      Description: checks if an event type is in the mailbox, if not returns 0
 */

int32_t dnp_event_request(uint32_t channel_id, uint32_t  mb_dir, dnp_event_t *event);

/*  dnp_poll: very important function of the driver
 *      Description: reads all CQ events, put in the events mailbox
 */
void dnp_poll();
#endif
