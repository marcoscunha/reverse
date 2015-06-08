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
#ifndef __RDMA_PKT_H
#define __RDMA_PKT_H


// Packet type definition
typedef enum rdma_pkt_types {
    RDMA_PKT_NONE, 
    RDMA_PKT_RDV_INIT, 
    RDMA_PKT_RDV_END,
    RDMA_PKT_EAGER     
} rdma_pkt_types_t;

// Internal classification of an Eager packet 
// (multi-packet handling)
typedef enum rdma_get_ret {
   RDMA_RETVAL_NONE = 0, 
   RDMA_RETVAL_DONE = 1, 
   RDMA_RETVAL_MULTI = 2,
} rdma_retval_t;

//Definition of EAGER and rendez-vous headers
typedef struct _pkt_rdv_eager {
    uint32_t buf_nwords;
} pkt_eager_t; 


typedef struct _pkt_rdv_init {
    uint32_t buf_address;
    uint32_t buf_nwords;
} pkt_rdv_init_t ;

typedef struct _pkt_rdv_end {
    uint32_t ack;
} pkt_rdv_end_t;

typedef struct _rdma_pkt {
    rdma_pkt_types_t pkt_type;
    uint32_t channel_id;
    uint32_t use_float;
    union {
        pkt_eager_t     eager_pkt;
        pkt_rdv_init_t  init_pkt;
        pkt_rdv_end_t   end_pkt;
    } pkt;
}  rdma_pkt_t;




#endif

