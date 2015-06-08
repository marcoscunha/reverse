#include <MemoryManager/MemoryManager.h>
#include <Processor/Processor.h>
#include <DnaTools/DnaTools.h>
#include <Core/Thread.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma"
#include <Private/debug.h>

#include <Private/Dnp.h>
#include <Private/Driver.h>
#include <Private/defines.h>

/* 
 *  Description: Called by the OS for a read operation on an RDMA point-to-point channel
 *                Waits a first packet to carry out either the Eager aither the Read-based
 *               Rendez-Vous protocol.
 */


status_t
dnp_rdma_write(void *handler, void *source,
	       int64_t offset ATTRIBUTE_UNUSED,
	       int32_t *p_count){

  DMSG("write\n");
  
  dnp_rdma_file_t *file = handler;
  rdma_channel_t  *rdma = file->channel;
  uint32_t         pkt_size = sizeof(rdma_pkt_t), to_write = (*p_count) >> 2;
  uint32_t         chunk, write_index = 0;
  rdma_pkt_t      *pkt1, *pkt2;
  uint32_t        *pkt_baseaddr, *src_baseaddr = (uint32_t *)source;

  if(file->status == CHANNEL_NOT_INITIALIZED){
    EMSG("Not initialized chanel, impossible to write !!\n");
    return DNA_ERROR;
  }
  
  if(to_write < rdma->eager_rdv_threshold){
		 /* 
		  * The number of data to send is below the threshold, sending of as
		  * many Eager packets as necessary to send the payload
		  */
        while(to_write){
			 /* 
			  * The quantity of data for the packet is either the max allowed
			  * payload, either what is left to send
			  */
			 chunk = (to_write > (DNP_MAX_PAYLOAD_SIZE >> 2)) ?
				  (DNP_MAX_PAYLOAD_SIZE >> 2) : to_write; 

			 /*
			  * Prepare the EAGER packet header
			  */
			 pkt1 = (rdma_pkt_t *)kernel_malloc(pkt_size + (chunk << 2), false);
			 pkt_baseaddr = (uint32_t *)pkt1;
			 pkt1->pkt_type   = RDMA_PKT_EAGER;
			 pkt1->channel_id = rdma->id;
			 pkt1->use_float  = rdma->use_float;
			 pkt1->pkt.eager_pkt.buf_nwords = chunk; 
			 
			/*
			 * Copy the data from src buffer with correct offset
			 * after the header 
			 */
			 dna_memcpy((uint32_t *)&pkt_baseaddr[pkt_size >> 2],
						&src_baseaddr[write_index], chunk << 2);
		 	 rdma_engine_send_pkt(rdma->tgt_rank, rdma->tgt_cpu_id, pkt1);
			 write_index += chunk;
			 to_write -= chunk;
        }
    }
    else
    {

		 /*
		  * + The number of data to send is above the threshold, Initiate the
		  *    Rendez-vous protocol
		  * + Register the src buffer to tell the HW that it will be a
		  *    target of a GET operation
		  * + Wait eventually if the register buffer zone of the device is
		  *    full. 
		  */
		 while (rdma_engine_register_buffer((int8_t *)source, *p_count,
											rdma->id)){
			  thread_yield();
		 }
		 
		 /*
		  * Prepare and send the INIT packet
		  */
		 pkt1 = (rdma_pkt_t *)kernel_malloc(pkt_size, false);
		 pkt1->pkt_type = RDMA_PKT_RDV_INIT;
		 pkt1->channel_id = rdma -> id;
		 pkt1->use_float = 0; // ints
		 pkt1->pkt.init_pkt.buf_address = (uint32_t)source;
		 pkt1->pkt.init_pkt.buf_nwords = (*p_count) >> 2; //!!

		 rdma_engine_send_pkt(rdma->tgt_rank, rdma->tgt_cpu_id, pkt1);

		 /*
		  * Wait END packet from reader
		  */
		 while (!rdma_engine_get_pkt(rdma->id, &pkt2, 1))  {
			  thread_yield();
		 }
		 
		 /*
		  * The read operation was completed, remove buffer from
		  * registered zone
		  */
		 rdma_engine_unregister_buffer(rdma->id);

		 /*
		  * Free the received END packet which was malloced in dnp_poll function
		  */
		 kernel_free(pkt2);
    }

    return DNA_OK;
}
