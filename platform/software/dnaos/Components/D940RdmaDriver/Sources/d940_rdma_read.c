/************************************************************************
 * Copyright (C) 2010 TIMA Laboratory                                    *
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



status_t d940_rdma_read(void *handler, void * destination, int64_t offset, int32_t * p_count) {
	channel_rdma_t * rdma =  handler; 	 
        rdma_pkt_t   *pkt1 = 0, *pkt2=0;
        uint32_t pkt_size = sizeof(rdma_pkt_t);
        uint32_t read_words = (*p_count) >> 2;
        uint32_t *pkt_baseaddr;
        int32_t to_read, write_index, header_nwords;
        uint32_t *dest_addr; 

#if 0;
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


// Wait reception of first packet from sender
        while (!dnp_rdma_get_pkt(rdma -> id, &pkt1))
        {
            thread_yield();
        }

        pkt_baseaddr = (uint32_t *)pkt1; 

        if (pkt1 -> pkt_type == RDMA_PKT_RDV_INIT)
        {
// Rendez-vous protocol, register the buffer to tell the HW that it will be a target of a GET operation
// Wait eventually if the register buffer zone of the device is full. 
            while ( dnp_rdma_register_buffer((int8_t *)destination,
                                            (int8_t *)(destination + (*p_count)), 
                                            rdma -> id                            )) 
            {
                thread_yield();
            }

// Start the get operation to pick up data from remote buffer
            dnp_rdma_get( destination, 
                          rdma -> tgt_rank, 
                          rdma -> tgt_cpu_id,  
                          (void *)pkt1 -> pkt . init_pkt . buf_address, 
                          ((*p_count) >> 2), rdma -> id ); 

// Wait completion of the GET operation
            while (!dnp_rdma_test(rdma -> id))
            {
                thread_yield(); 
            }        

// Send the end packet of the Rendez-vous protocol
            pkt2 =  (rdma_pkt_t *) malloc (pkt_size);
            pkt2 -> pkt_type = RDMA_PKT_RDV_END;
            pkt2 -> channel_id = rdma -> id;
            pkt2 -> use_float = 0; // ints only
            pkt2 -> pkt . end_pkt . ack = 1; 
            dnp_rdma_send_pkt(rdma -> tgt_rank, rdma -> tgt_cpu_id, pkt2);

// Unregister the buffer and free the received init pkt which was malloced in the dnp_poll
// function
            dnp_rdma_unregister_buffer(rdma ->id);
            free(pkt1);
        }
	else // EAGER
	{
                to_read = (*p_count >> 2);
                write_index = 0, header_nwords = (sizeof(rdma_pkt_t) >> 2);
		*dest_addr = (uint32_t *)destination; 

// Copy the contents of the EAGER pkt payload into the destination buffer
		memcpy(dest_addr, &pkt_baseaddr[header_nwords], pkt1 -> pkt . eager_pkt . buf_nwords << 2);
                to_read -=   pkt1 -> pkt . eager_pkt . buf_nwords;
                write_index +=  pkt1 -> pkt . eager_pkt . buf_nwords;

// Release the pkt which was malloced in the dnp_poll function
                free(pkt1);

// This loop treats the sending of large messages (over the DNP MTU) 
                /*
                 *  Caution! If the reader wants more data than the receiver has to offer, this could result
                 *  in the reading of the following eager packet
                 */
                while (to_read  > 0)
                {
                    while (!dnp_rdma_get_pkt(rdma -> id, &pkt1))
                    {
                        thread_yield();
                    }
                    pkt_baseaddr = (uint32_t *)pkt1; 

// Copy the contents of the eager packet to the destination buffer with correct offset
                    memcpy(&dest_addr[write_index], &pkt_baseaddr[header_nwords], pkt1 -> pkt . eager_pkt . buf_nwords << 2);
                    to_read -=   pkt1 -> pkt . eager_pkt . buf_nwords;
                    write_index +=  pkt1 -> pkt . eager_pkt . buf_nwords;

// Release the pkt which was malloced in the dnp_poll function
                    free(pkt1);
                }
        }

#if 0
// Release hw-lock
        cpu_write(UINT32, (AT91_MAGIC_REG_BASE + (AT91_REG_MGCMUTEXCTL << 2)), spin_check);
        cpu_trap_restore(it_status);
#endif

        return DNA_OK;
;
}



