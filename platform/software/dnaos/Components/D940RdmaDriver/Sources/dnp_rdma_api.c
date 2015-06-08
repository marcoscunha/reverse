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
#include <stdlib.h>
#include <string.h>
#include <Private/Dnp.h>



// Initialize hardware-dependant part of the driver
void dnp_rdma_init()
{
// Initialize topological settings in DNP
    dnp_topo_init();
// Initialise events
    dnp_event_init();
}


void dnp_rdma_open(uint32_t channel_id)
{
    dnp_event_open(channel_id);
}


// Register a streaming buffer for PUT operations
int32_t dnp_register_streaming_buffer(int8_t *startptr, int8_t *endptr, uint32_t magic)
{
    dnp_lut_entry_t lutentry;

    // Create LUT entry in DNP
    lutentry . start_addr = startptr;
    lutentry . end_addr = endptr;
    lutentry . magicw = magic; 
    lutentry . flags = MSK_LUT_FLAG_VALID | MSK_LUT_FLAG_STREAMING_BUFFER; 

    dnp_send_lut_entry(&lutentry, 0 );
    return 0;
}


// Register a regular  src or target buffer zone for a GET operation
int32_t dnp_rdma_register_buffer(int8_t *startptr, int8_t *endptr, uint32_t magic)
{
    uint32_t i;
    dnp_lut_entry_t lut;

//search for a free slot in luts
    for ( i = 1; i < DNP_RDMA_LUT_NENTRIES; i++ )
    {
        dnp_read_lut_entry(&lut, i);
        if (!(lut . flags & MSK_LUT_FLAG_VALID))
            break;
    }

    if (i == DNP_RDMA_LUT_NENTRIES)
        return -1;
    
    dnp_lut_entry_t lutentry;

    // Create LUT entry in DNP
    lutentry . start_addr = startptr;
    lutentry . end_addr = endptr;
    lutentry . magicw = magic ; 
    lutentry . flags =  MSK_LUT_FLAG_VALID; 

    dnp_send_lut_entry(&lutentry, i);
    return 0;
}


// Unregister regular buffers
int32_t dnp_rdma_unregister_buffer(uint32_t magic)
{
    uint32_t i;
    dnp_lut_entry_t  lut,lutentry = {0,0,0,0};
//search for channel id in magic word of luts
    for ( i = 1; i < DNP_RDMA_LUT_NENTRIES; i++ )
    {
        dnp_read_lut_entry(&lut, i);
        if (lut . magicw == magic)
            break;
    }
    if (i == DNP_RDMA_LUT_NENTRIES)
        return -1;

    dnp_send_lut_entry(&lutentry, i);
    
    return 0;
}


// Check completion of GET request
uint32_t dnp_rdma_test(uint32_t channel_id)
{
    dnp_event_t event;
    // not called by a separate thread
    dnp_poll();

    if (dnp_event_request(channel_id, MAILBOX_IN, &event) && (event . type == RDMA_EVENT_TYPE_GET_RSP))
    {
        dnp_debug("hw_rdma_get_test: got a get resp.\r\n");
        return 1;
    }
    else
        return 0;
}




// Get a rdma pkt, this method is non-blocking to allow the generic part
// to yield the task
int32_t dnp_rdma_get_pkt(uint32_t channel_id, rdma_pkt_t **pkt)
{
    dnp_event_t event;
 
    // not called by a separate thread
    dnp_poll();

// Failsafe: drop all non expected packets on the channel
    do {  dnp_event_request(channel_id, MAILBOX_IN, &event); }
    while ((event . type != RDMA_EVENT_TYPE_NONE) && (event . pkt_type == RDMA_PKT_NONE) );


    if (event . type == RDMA_EVENT_TYPE_PUT_REQ)
    {
        dnp_debug("dnp_rdma_get_pkt: got one packet. %d %d %d %d\r\n",
                (int)event . pkt ->  pkt_type,
                (int)event . pkt ->  channel_id,
                (int)event . pkt ->  pkt . init_pkt . buf_address,
                (int)event . pkt ->  pkt . init_pkt . buf_nwords
                );

        *pkt =  event . pkt;
        return RDMA_RETVAL_DONE;

    }
    else
    {
        return RDMA_RETVAL_NONE;
    }
}


// Send a rdma pkt using a PUT targeting a streaming buffer
int32_t dnp_rdma_send_pkt(uint32_t rank, uint32_t dev,rdma_pkt_t *pkt)
{
    dnp_command_t cmd;
    cart_coords_t local_coords, dest_coords;
    uint32_t srcx, srcy, srcz, dstx, dsty, dstz;
    uint32_t pkt_size = sizeof(rdma_pkt_t);
    dnp_event_t event;
    rdma_pkt_t *pkt_header = (rdma_pkt_t *)pkt;

// Rank -> 3D coords conversion
    dnp_cart_get_local_coords(&local_coords);
    dnp_cart_coords(rank, &dest_coords);

    srcx = dnp_cart_get_dim((uint32_t)0, &local_coords);
    srcy = dnp_cart_get_dim((uint32_t)1, &local_coords);
    srcz = dnp_cart_get_dim((uint32_t)2, &local_coords);
    dstx = dnp_cart_get_dim((uint32_t)0, &dest_coords);
    dsty = dnp_cart_get_dim((uint32_t)1, &dest_coords);
    dstz = dnp_cart_get_dim((uint32_t)2, &dest_coords);

// word0
    cmd . dstx    =  dstx; 
    cmd . dsty    =  dsty;
    cmd . dstz    =  dstz; 
    cmd . dstdev  = dev;                         

// word1
    cmd . len  = ( pkt_header -> pkt_type == RDMA_PKT_EAGER) ? pkt_size + (pkt_header -> pkt . eager_pkt . buf_nwords << 2): pkt_size;
    dnp_debug("Sending  packet of %d bytes from (%d,%d,%d) to (%d %d %d)\r\n", (int)cmd . len , (int)srcx, (int)srcy, (int)srcz ,(int)dstx, (int)dsty, (int)dstz);

// word2
    cmd . srcx    = srcx; 
    cmd . srcy    = srcy; 
    cmd . srcz    = srcz; 
    cmd . srcdev  = RDMA_COMMON . local_cpu_id;                            

//word3
// generate an event for each paquet (clustered)
// NB: SW takes care of not exceeding the max payload of the DNP
    cmd . broadcast = 0 ;
    cmd . comp      = (COMP_CQ | COMP_CQ_FRAG | COMP_RDMA_EVENT);
    cmd . cmd       = DNP_CMD_RDMA_PUT; 

//word4
// write ptr of the buffer for source address
    cmd . src_address = (uint32_t) pkt ; 

// word 5
 // destination address: streaming buffer
    cmd . dst_address = DNP_STREAMING_BUFFER_MAGIC; 

// word6
    cmd . magicw = pkt_header -> channel_id; //magic 

    dnp_send_cmd(&cmd);

    // Wait HW acknowledgement
    while (!dnp_event_request(cmd . magicw , MAILBOX_OUT, &event)) dnp_poll();

    dnp_debug("Sending packet done\r\n");
    free(pkt);
    return 0;
}

// Initiate a GET request to peek data from remote tile
int32_t dnp_rdma_get(void *dst, uint32_t dest_rank, uint32_t dest_dev, void *src, uint32_t nwords, uint32_t channel_id)
{
    dnp_command_t cmd;
    dnp_event_t event;
    cart_coords_t local_coords, dest_coords;
    uint32_t srcx, srcy, srcz, dstx, dsty, dstz;

// Rank -> 3D coordinates conversion
    dnp_cart_get_local_coords(&local_coords);
    dnp_cart_coords(dest_rank, &dest_coords);

    srcx = dnp_cart_get_dim((uint32_t)0, &local_coords);
    srcy = dnp_cart_get_dim((uint32_t)1, &local_coords);
    srcz = dnp_cart_get_dim((uint32_t)2, &local_coords);
    dstx = dnp_cart_get_dim((uint32_t)0, &dest_coords);
    dsty = dnp_cart_get_dim((uint32_t)1, &dest_coords);
    dstz = dnp_cart_get_dim((uint32_t)2, &dest_coords);

    dnp_debug("Getting %d words from (%d %d %d) to (%d %d %d)\r\n", (int)nwords, (int)dstx, (int)dsty, (int)dstz, (int)srcx, (int)srcy, (int)srcz);


// word0
    cmd . dstx    =  srcx; 
    cmd . dsty    =  srcy;
    cmd . dstz    =  srcz; 
    cmd . dstdev  =  RDMA_COMMON . local_cpu_id;                         

// word1
    cmd . len  = nwords << 2;

// word2
    cmd . srcx    = dstx; 
    cmd . srcy    = dsty; 
    cmd . srcz    = dstz; 
    cmd . srcdev  = dest_dev; 
//word3
// generate an event for each paquet (clustered)
    cmd . broadcast = 0 ;
//FIXME: DNP should be able to generate completion at the end of the GET 
// (not the intermediary packets)
    cmd . comp      = COMP_CQ | COMP_RDMA_EVENT; // LAST_PACKET
    cmd . cmd       = DNP_CMD_RDMA_GET; 

//word4
// write ptr of the buffer for source address
    cmd . src_address = (uint32_t) src ; 

// word 5
 // destination address: streaming buffer
    cmd . dst_address = (uint32_t) dst; 

// word6
    cmd . magicw = channel_id; //magic 


    dnp_send_cmd(&cmd);

  // Wait HW acknowledgement of the GET request 
    while (!dnp_event_request(channel_id, MAILBOX_OUT, &event)) dnp_poll();

    dnp_debug("Get done\r\n");
    return nwords;

}

