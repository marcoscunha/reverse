/************************************************************************
 * Copyright (C) 2008 TIMA Laboratory                                    *
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
#include <Processor/Processor.h>
#include <DnaTools/DnaTools.h>
#include <Private/RdmaChannel.h>
#include <Private/Dnp.h>
#include "magicV_regs.h"

/* 
 *  Description: Called by the OS for a read operation on an RDMA point-to-point channel
 *                Waits a first packet to carry out either the Eager aither the Read-based
 *               Rendez-Vous protocol.
 */


status_t d940_rdma_write (void * handler, void * source, int64_t offset, int32_t * p_count) {
    channel_rdma_t * rdma = handler;
    rdma_pkt_t   *pkt1, *pkt2;
    uint32_t pkt_size = sizeof(rdma_pkt_t), to_write = (*p_count) >> 2, chunk, write_index=0;
    uint32_t *pkt_baseaddr, *src_baseaddr=(uint32_t *)source;

#if 0
    uint32_t spin_mask, spin_check, res = 0;
    interrupt_status_t it_status = 0;

// Take hardware lock
    it_status = cpu_trap_mask_and_backup();
    spin_mask =   RDMA_COMMON . hw_spinlock;
    spin_check =  RDMA_COMMON . hw_spinlock<< 16;
    do
    {
        cpu_write(UINT32, (AT91_MAGIC_REG_BASE + (AT91_REG_MGCMUTEXCTL << 2)), spin_mask);
        cpu_read(UINT32, (AT91_MAGIC_REG_BASE + (AT91_REG_MGCMUTEXCTL << 2)), & res);
    }
    while (! ((res & spin_mask) && (res & spin_check)));

#endif


    if ( to_write < rdma -> eager_rdv_threshhold)
    {
// The number of data to send is below the threshhold, seding of as many Eager packets as necessary
// to send the payload
        while (to_write)
        {
// THe quantity of data for the packet is either the max allowed payload, either what is left to send
            chunk = (to_write > (HW_MAX_PAYLOAD_SIZE >> 2)) ? HW_MAX_PAYLOAD_SIZE >> 2: to_write; 
//Prepare the EAGER packet header
            pkt1 =          (rdma_pkt_t *) malloc (pkt_size + (chunk << 2));
            pkt_baseaddr =  (uint32_t *)pkt1;
            pkt1 -> pkt_type = RDMA_PKT_EAGER;
            pkt1 -> channel_id = rdma -> id;
            pkt1 -> use_float = rdma -> use_float;
            pkt1 -> pkt . eager_pkt . buf_nwords = chunk; 
//Copy the data from src buffer with correct offset after the header 
            memcpy((uint32_t *)&pkt_baseaddr[pkt_size >> 2], &src_baseaddr[write_index], chunk << 2);
            hw_rdma_send_pkt(rdma -> tgt_rank, rdma -> tgt_cpu_id , pkt1);
            write_index += chunk;
            to_write -= chunk;
        }
    }
    else
    {

// The number of data to send is above the threshhold, Initiate the Rendez-vous protocol
// Register the src buffer to tell the HW that it will be a target of a GET operation
// Wait eventually if the register buffer zone of the device is full. 
        while (hw_rdma_register_buffer((int8_t *)source,(int8_t *)(source + (*p_count)), rdma -> id))
        {
            thread_yield();
        }

// Prepare and send the INIT packet
        pkt1 =  (rdma_pkt_t *) malloc (pkt_size);
        pkt1 -> pkt_type = RDMA_PKT_RDV_INIT;
        pkt1 -> channel_id = rdma -> id;
        pkt1 -> use_float = 0; // ints
        pkt1 -> pkt . init_pkt . buf_address = (uint32_t)source;
        pkt1 -> pkt . init_pkt . buf_nwords = (*p_count) >> 2; //!!

        hw_rdma_send_pkt(rdma -> tgt_rank, rdma -> tgt_cpu_id, pkt1);

// Wait END packet from reader
        while (!hw_rdma_get_pkt(rdma -> id, &pkt2))  {
            thread_yield();
        }
// The read operation was completed, remove buffer from registered zone
        hw_rdma_unregister_buffer(rdma ->id);

// Free the received END packet which was malloced in dnp_poll function
        free(pkt2);
    }

#if 0
// Release hw-lock
    cpu_write(UINT32, (AT91_MAGIC_REG_BASE + (AT91_REG_MGCMUTEXCTL << 2)), spin_check);
    cpu_trap_restore(it_status);
#endif

    return DNA_OK;
}
