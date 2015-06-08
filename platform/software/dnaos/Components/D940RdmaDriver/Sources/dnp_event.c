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

// This file deals with the completion queue of the DNP. event_handler
// reads all available fields in ring buffer, then fills the
// corresponding event mailbox. 

#include <stdlib.h>
#include <string.h>

#include <Processor/Processor.h>
#include <Private/Dnp.h>
#include <Private/RdmaChannel.h>

#define DNP_POISON_WORD    0xBAADFACE

// streaming buffer and completion queue
//static uint32_t dnp_compqueue[DNP_COMPQUEUE_SIZE];
//static uint8_t  streaming_buffer[sizeof(rdma_pkt_t) +  DNP_MAX_PAYLOAD_SIZE];



/*  dnp_event_init
 *      Description: initializes  the DNP RB, and  the mailboxes
 */
void dnp_event_init()
{
    uint32_t rb_sa;
    dnp_reg_read(REG_DNP_RBSA, &rb_sa); 
    
// allocate mailboxes to hold DNP completion queue events
    for (uint32_t id=0; id < RDMA_CHANNEL_NDEV; id++)
        dnp_mailbox_allocate(RDMA_CHANNELS[id] . id); 
    
// Verify that DNP init wasn't already carried out 
    if (rb_sa != (uint32_t)DNP_SETTINGS . compqueue_buffer)
    {
// Poison the completion queue area
        for (int i=0; i< DNP_SETTINGS . compqueue_size ; i++)
            DNP_SETTINGS . compqueue_buffer[i] = DNP_POISON_WORD;

// Inidcate to the DNP te completion queue buffer
        dnp_reg_write(REG_DNP_RBSA,  DNP_SETTINGS . compqueue_buffer);
        dnp_reg_write(REG_DNP_RBEA, ((uint32_t)(DNP_SETTINGS . compqueue_buffer) + DNP_SETTINGS . compqueue_size - 1));
        dnp_reg_write(REG_DNP_RBNR,  DNP_SETTINGS . compqueue_buffer);
        dnp_reg_write(REG_DNP_RBNW,  DNP_SETTINGS . compqueue_buffer);

// restart dnp's RB manager (required to take the new values into account)
        dnp_reg_write((DNP_REG *)REG_DNP_FLAG_RB_RESTART, (DNP_REG)MSK_DNP_FLAG_RB_RESTART);

// Clear restart flag of the RB_RESTART register
        dnp_reg_write(REG_DNP_FLAG_RB_RESTART, 0);


// Declare streaming buffer, to receive PUT requests
        dnp_register_streaming_buffer((int8_t *)DNP_SETTINGS . streaming_buffer,
                (int8_t *)(DNP_SETTINGS . streaming_buffer + sizeof(rdma_pkt_t) + DNP_MAX_PAYLOAD_SIZE), 0xDEADBEEF); 
    }
}


/*  dnp_event_open
 *      Description:  
 */
void dnp_event_open(uint32_t channel_id)
{
}



/*  dnp_event_get_request
 *      Description: gets first available event in mailbox 
 */
int32_t dnp_event_request(uint32_t channel_id, uint32_t  mb_dir, dnp_event_t *event)
{
    if (!dnp_mailbox_is_empty(channel_id, mb_dir)) 
    {
        dnp_debug("dnp_event_request: event found in the mailbox %d in %s direction\r\n", (int)channel_id, mb_dir? "output":"input");
        dnp_mailbox_pop_mail(channel_id, event, mb_dir);
        dnp_debug("dnp_event_request: len: %d\r\n", (int)event -> len);
        return 1;
    }
    else
    {
        event -> type = RDMA_EVENT_TYPE_NONE;
        return 0;
    }

    return 0;
}



/*  dnp_poison_compentry(uint32_t *);
 *  poisons a compqueue entry . Useful when for AHB trafic reasons, the RB next write
 *  is set before completion word is available
 */
void dnp_poison_compentry(uint32_t * compentry)
{
    for (int i=0; i<DNP_COMPLETION_NWORDS;i++)
    {
        CPU_WRITE(UINT32,&compentry[i],DNP_POISON_WORD);
    }
}

/*  dnp_poll: very important function of the driver
 *      Description: reads all CQ events, put in the events mailbox
 */
void dnp_poll()
{
    cart_coords_t src_coords, dest_coords, local_coords; 
    uint32_t flag_rb_full, rb_sa, rb_ea, rb_nr, rb_nw;
    uint32_t srcx,srcy,srcz,dstx,dsty,dstz;
    uint8_t cmd;
    dnp_completion_event_t completion_event;
    uint32_t *completion_base = (uint32_t *)&completion_event;
    dnp_event_t event;
    dnp_mailbox_direction_t dir = MAILBOX_IN;

// Verify in rinb buffer area of the dnp
    dnp_reg_read(REG_DNP_FLAG_RB_FULL, &flag_rb_full);
    dnp_reg_read(REG_DNP_RBNR, &rb_nr); 
    dnp_reg_read(REG_DNP_RBNW, &rb_nw);

    dnp_debug("dnp_event_handler: 0x%x, 0x%x, 0x%x\r\n", (int)flag_rb_full,  (int)rb_nr, (int)rb_nw);
    if ( !(flag_rb_full & MSK_DNP_FLAG_RB_FULL) &&
           (rb_nr == rb_nw )  )
    {   
        return;
    } 
    dnp_reg_read(REG_DNP_RBSA, &rb_sa); 
    dnp_reg_read(REG_DNP_RBEA, &rb_ea);
     

// read each completion queue event in system memory, put it in the good mailbox
    while ((rb_nr != rb_nw) || (flag_rb_full & MSK_DNP_FLAG_RB_FULL))
    {
        cpu_vector_read(UINT32, &completion_event, rb_nr, DNP_COMPLETION_NWORDS);
        dnp_debug("dnp_event_handler: ***** new completion\r\n");
        dnp_debug("dnp_event_handler: dstx: 0x%x, dsty: 0x%x, dstz: 0x%x, dstdev: 0x%x cflags: 0x%x\r\n", 
                (int) completion_event . dstx, (int) completion_event . dsty,  (int) completion_event . dstz, (int) completion_event . dstdev, (int) completion_event . cflags);
        dnp_debug("dnp_event_handler: srcx: 0x%x, srcy: 0x%x, srcz: 0x%x, srcdev: 0x%x, len: 0x%x\r\n",
                (int) completion_event . srcx, (int) completion_event . srcy,  (int) completion_event . srcz, (int) completion_event . srcdev , (int) completion_event . len);
        dnp_debug("dnp_event_handler: rsrcx:: 0x%x, rsrcy: 0x%x, rsrcz: 0x%x, len: 0x%x, tgtdev: 0x%x, cmd: 0x%x \r\n",
                (int) completion_event . rdma_srcx,  (int) completion_event . rdma_srcy, (int) completion_event . rdma_srcz, (int) completion_event . rdma_len, (int) completion_event . tgtdev,  (int) completion_event . cmd);
        dnp_debug("dnp_event_handler: src vaddress: 0x%x \r\n",(int) completion_event . src_vaddress);
        dnp_debug("dnp_event_handler: dst vaddress: 0x%x \r\n",(int) completion_event . dst_vaddress);
        dnp_debug("dnp_event_handler: crc: 0x%x, nhops: 0x%x, err: 0x%x \r\n", (int) completion_event . crc, (int) completion_event . nhops,  (int) completion_event . err);
        dnp_debug("dnp_event_handler: lut flags: 0x%x \r\n",(int) completion_event . lflags);
        dnp_debug("dnp_event_handler: magicw: 0x%x \r\n",(int) completion_event . magicw);

// Verify if the completion event has been received totally thans to the poison word
	if ((completion_base[0] == DNP_POISON_WORD) | (completion_base[1] == DNP_POISON_WORD) | (completion_base[2] == DNP_POISON_WORD) | (completion_base[3] == DNP_POISON_WORD)
               | (completion_base[4] == DNP_POISON_WORD) | (completion_base[5] == DNP_POISON_WORD) | (completion_base[6] == DNP_POISON_WORD) | (completion_base[7] == DNP_POISON_WORD)) 
		continue;

        cmd = completion_event . cmd;

// Make 3D coordinates -> rank conversion
        dnp_cart_get_local_coords(&local_coords);

        dstx =  completion_event . dstx;
        dsty =  completion_event . dsty;
        dstz =  completion_event . dstz;
        dnp_cart_set_dim(dstx, 0, &dest_coords);
        dnp_cart_set_dim(dsty, 1, &dest_coords);
        dnp_cart_set_dim(dstz, 2, &dest_coords);

        srcx =  completion_event . srcx; 
        srcy =  completion_event . srcy;
        srcz =  completion_event . srcz;
        dnp_cart_set_dim(srcx, 0, &src_coords);
        dnp_cart_set_dim(srcy, 1, &src_coords);
        dnp_cart_set_dim(srcz, 2, &src_coords);


// Verify if the first available event is meant for this CPU. If not, yield. 
// This means that the processor waits for another processor to read its completion event
// to continnune the process
        if (cart_coordscmp(&src_coords, &local_coords))  // ack of outgoing operation
        {

            if ( completion_event . srcdev != RDMA_COMMON . local_cpu_id )
            {
                return;
            }
        }
        else
        {
            if (completion_event . dstdev != RDMA_COMMON . local_cpu_id )
            {
                return;
            }
        }



// According to the command of the CQ entry, fill in the correct mailbox
// wit the driver internal event representation
        switch (cmd)
        {
            case DNP_PKT_HDR_RDMA_PUT:
                dnp_debug("dnp_event_handler: src coords: (%d,%d,%d) dest coords:(%d,%d,%d)\r\n", 
                        (int)srcx, (int)srcy,(int)srcz,(int)dstx,(int)dsty,(int)dstz);


                if (cart_coordscmp(&src_coords, &local_coords))
                {
// Outgoing PUT operation acknowledgment
                    dnp_debug("dnp_event_handler: event type PUT_ACK in mailbox\r\n");
                    event . type = RDMA_EVENT_TYPE_PUT_ACK;
                    event . len  = completion_event . rdma_len;
                    event . ptr  = completion_event . src_vaddress; //source

                    if (completion_event . dst_vaddress == (uint32_t)DNP_STREAMING_BUFFER_MAGIC)
                        // rendez-vous packet: channel id in the packet itself
                    {
                        event . pkt_type =    ((rdma_pkt_t *) event . ptr) -> pkt_type;
                        event . channel_id =  ((rdma_pkt_t *) event . ptr) -> channel_id;
                    }
                    else
                    // real PUT operation acknowledgement, channel id int the magic word
                    {
                        event . pkt_type = RDMA_PKT_NONE;
                        event . channel_id = (completion_event . magicw);
                    }

                    dnp_debug("dnp_event_handler: channel: %d len: %d ptr: 0x%x \r\n", (int)event . channel_id, (int)event.len, (int)event.ptr);
                    dir = MAILBOX_OUT;
                }
                else if (cart_coordscmp(&dest_coords, &local_coords))
                {

// Ingoing PUT operation notice
                    event . type =  RDMA_EVENT_TYPE_PUT_REQ;
                    event . len  =  completion_event . rdma_len;
                    event . ptr  =  completion_event . dst_vaddress; // dst

                    if (completion_event . magicw == 0xDEADBEEF)
                    {
// The PUT operation targeted a streaming buffer
                        event . pkt_type = ((rdma_pkt_t *) event . ptr)    -> pkt_type;
                        event . channel_id =  ((rdma_pkt_t *) event . ptr) -> channel_id;
                        event . pkt = (rdma_pkt_t *) malloc (event . len);
// Copy the streaming buffer contents into the malloced area
// FIXME: this intermediate copy could be avoided to increase performance
// by having an array of streaming buffers
                        memcpy(event . pkt, DNP_SETTINGS . streaming_buffer, event . len);

                        dir = MAILBOX_IN;

// The streaming buffer has been read, it can be the target of a new PUT operation 
                        dnp_register_streaming_buffer((int8_t *)DNP_SETTINGS . streaming_buffer,
                                (int8_t *)(DNP_SETTINGS . streaming_buffer + sizeof(rdma_pkt_t) + DNP_MAX_PAYLOAD_SIZE), 
                                0xDEADBEEF); 
                    }
                    else
                    {
// A normal PUT operation on a registered buffer, this is not used in the current driver
                        dnp_debug("dnp_event_handler:  ERROR non supported event type PUT in mailbox %d\r\n", (int)completion_event . magicw);
                        event . pkt_type = RDMA_PKT_NONE;
                        event . channel_id = (completion_event . magicw);
                        dir = MAILBOX_IN;

                    }
                    dnp_debug("dnp_event_handler: channel:%d len: %d ptr: 0x%x \r\n", (int)event.channel_id, (int)event.len, (int)event.ptr);
                }

                break;

            case DNP_PKT_HDR_RDMA_GET_REQ:
// Outgoing GET operation acknowledgment
// This comes at the very end of a GET operation on reader side, 
// When the payload exceeds the maximum packet size, it is triggered once the very last 
// packet has been received
                dnp_debug("dnp_event_handler: event type GET_REQ in mailbox\r\n");
                event . type = RDMA_EVENT_TYPE_GET_ACK;
                event . len  = completion_event . rdma_len;
                event . ptr  = completion_event . src_vaddress; //source
                event . channel_id  = completion_event . magicw; 
                dir = MAILBOX_OUT;
                break;

            case DNP_PKT_HDR_RDMA_GET_RESP:
// In coming or outgoing GET acknowledgement. These events come after each emission/reception
// of GET operation packets
                dnp_debug("dnp_event_handler: GET RESP  src coords: (%d,%d,%d) dest coords:(%d,%d,%d)\r\n", 
                        (int)srcx, (int)srcy,(int)srcz,(int)dstx,(int)dsty,(int)dstz);
                if (cart_coordscmp(&src_coords, &local_coords))
                {
// Outgoing get ack on sender side
                    dnp_debug("dnp_event_handler: event type GET_ACK in mailbox\r\n");
                    event . type = RDMA_EVENT_TYPE_GET_ACK;
                    event . len  = completion_event . rdma_len;
                    event . ptr  = completion_event . src_vaddress; //source
                    event . channel_id  = completion_event . magicw; 
                    dir = MAILBOX_OUT;
                 }
                else if (cart_coordscmp(&dest_coords, &local_coords))
                {
// Outgoing get ack on receiver side
                    dnp_debug("dnp_event_handler: event type GET_RESP in mailbox %d\r\n", (int)completion_event . magicw);
                    event . type =  RDMA_EVENT_TYPE_GET_RSP;
                    event . len  =  completion_event . rdma_len;
                    event . ptr  =  completion_event . dst_vaddress;
                    event . channel_id  = completion_event . magicw; 
                    dir = MAILBOX_IN;
                }
                dnp_debug("dnp_event_handler: GET_RESP channel:%d len: %d ptr: 0x%x dir: %d \r\n", (int)event . channel_id, (int)event . len, (int)event . ptr, (int)dir);
                break;
            default:
                        dnp_debug("dnp_event_handler: Bad RDMA type received\r\n");
                        break;

            }

// So far , we have a DNP event, we can push it into the correct canel mailbox
            dnp_mailbox_push_mail(event . channel_id , &event, dir);


// poison treated completion entry
	    dnp_poison_compentry((uint32_t *)rb_nr);

// update rb pointer and flag
            rb_nr = rb_sa + (rb_nr - rb_sa + SIZEOF_CQ_STRUCTURE*4 )  %  (rb_ea - rb_sa + 1);
            flag_rb_full = flag_rb_full & ~MSK_DNP_FLAG_RB_FULL; //clear flag

// update dnp rb registers
            dnp_reg_write(REG_DNP_FLAG_RB_FULL, flag_rb_full); 
            dnp_reg_write(REG_DNP_RBNR, rb_nr); 
                  
            if (dnp_mailbox_is_full(event . channel_id, dir)) 
            {
                dnp_debug("dnp_event_handler: warning, MB %d  full\r\n", (int)event . channel_id);
                break;  
            }
        }
}


