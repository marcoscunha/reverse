/* *************************************************************************
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
#ifndef __DNP_H
#define __DNP_H

//#define DNP_DEBUG

#include <stdint.h>

#ifdef DNP_DEBUG
#include <stdio.h>
#define dnp_debug(FMT, ARGS...) printf(FMT, ## ARGS)
#else
#define dnp_debug(FMT, ARGS...)
#endif

#include "Private/dnp_api.h"
#include "Private/dnp_event.h"
#include "Private/dnp_mailbox.h"
#include "Private/dnp_types.h"
#include "Private/dnp_rdma_pkt.h"


// dNP base address in AHB bus mappping 
#define DNP_AHB_OFFSET 0x700000


// Configuration structure filled at linker script level
// NOTICE: this structure is shared between the processors of the tile

typedef struct dnp_settings
{
// Completion buffer physical address
    uint32_t *compqueue_buffer;
// Completion buffer size
    uint32_t  compqueue_size;
// Streaming buffer physical address
    uint32_t *streaming_buffer;
// Streaming buffer size
    uint32_t  streaming_size;
} dnp_settings_t;

extern dnp_settings_t DNP_SETTINGS;



// API between the hardware dependant part of the driver and the generic one

// Register a regular  src or target buffer zone for a GET operation
int32_t dnp_rdma_register_buffer(int8_t *startptr, int8_t *endptr, uint32_t magic);
// Register a streaming buffer for PUT operations
int32_t dnp_register_streaming_buffer(int8_t *startptr, int8_t *endptr, uint32_t magic);
// Unregister regular buffers
int32_t dnp_rdma_unregister_buffer(uint32_t channel_id);

// Initialize hardware-dependant part of the driver
void     dnp_rdma_init();
// Open a channel
void     dnp_rdma_open(uint32_t channel_id);
// Get a rdma pkt, thhis method is non-blocking to allow the generic part
// to yield the task
int32_t  dnp_rdma_get_pkt(uint32_t channel_id, rdma_pkt_t **pkt);
// Send a rdma pkt using a PUT targeting a streming buffer
int32_t  dnp_rdma_send_pkt(uint32_t  rank, uint32_t dev, rdma_pkt_t *pkt);
// Initiate a GET request to peek data from remote tile
int32_t  dnp_rdma_get(void *dst, uint32_t dest_rank, uint32_t dest_dev,   void *src, uint32_t nwords, uint32_t channel_id);
// Check completion of GET request
uint32_t dnp_rdma_test(uint32_t channel_id);

#endif
