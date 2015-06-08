#include <MemoryManager/MemoryManager.h>
#include <Processor/Processor.h>
#include <DnaTools/DnaTools.h>
#include <Core/Thread.h>

#include <Private/Dnp.h>
#include <Private/Driver.h>

#include <Private/defines.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma:read"
#include <Private/debug.h>

/* 
 *  Description: Called by the OS for a read operation on an RDMA point-to-point channel
 *                Waits a first packet to carry out either the Eager aither the Read-based
 *               Rendez-Vous protocol.
 */

status_t
dnp_rdma_read(void *handler, void *destination,
	      int64_t offset ATTRIBUTE_UNUSED,
	      int32_t *p_count) {
 
  DMSG("read (%d)\n", *p_count);

	 dnp_rdma_file_t *file = handler;
	 rdma_channel_t  *rdma = file->channel; 	 
	 rdma_pkt_t      *pkt1 = 0, *pkt2 = 0;
	 uint32_t        *pkt_baseaddr;
	 uint32_t         pkt_size = sizeof(rdma_pkt_t);
	 int32_t          write_index, header_nwords;
	 uint32_t        *dest_addr= (uint32_t *)destination; 
	 int32_t          to_read = (*p_count >> 2);

	 if(file->status == CHANNEL_NOT_INITIALIZED){
	   EMSG("Not initialized chanel, impossible to read !!\n");
	   return DNA_ERROR;
	 }

	if(file->status == CHANNEL_RIP){
		 if(to_read > file->lbuf_size){
			  EMSG("Reading more data that remaining in the buffer\n");
			  return DNA_ERROR;
		 }else{
			  DMSG("reading in the buffer %d (%d/%d)!!!\n",
					to_read, file->lbuf_pos, file->lbuf_size);

			  dna_memcpy(dest_addr, file->lbuffer + file->lbuf_pos,
						 to_read << 2);
			  
			  file->lbuf_size -= to_read;
			  file->lbuf_pos  += to_read;

			  if(file->lbuf_size == 0)
				   file->status = CHANNEL_READY;
		 }
		 return DNA_OK;
	}

	/*
	 * Wait reception of first packet from sender
	 */
	/*
	 * To be impoved with interups and more integrated non-blocking
	 */
	if(rdma->non_blocking){
		 int32_t ret = rdma_engine_get_pkt(rdma->id, &pkt1, 0);
		 if(ret == RDMA_RETVAL_NONE){
			  *p_count = 0;
			  return DNA_OK;
		 }
 	}else{
		 while (!rdma_engine_get_pkt(rdma->id, &pkt1, 1)){
			  thread_yield();
		 }
	}
	pkt_baseaddr = (uint32_t *)pkt1; 

	if (pkt1->pkt_type == RDMA_PKT_RDV_INIT){

		 /*
		  * + Rendez-vous protocol, register the buffer to tell the HW
		  *     that it will be a target of a GET operation
		  * + Wait eventually if the register buffer zone of the device 
		  *     is full. 
		  */
		 while(rdma_engine_register_buffer((int8_t *)destination,
										   *p_count, 
										   rdma->id)){
			  thread_yield();
		 }

		 /*
		  * Start the get operation to pick up data from remote buffer
		  */
		 rdma_engine_get(destination, 
						 rdma->tgt_rank, 
						 rdma->tgt_cpu_id,  
						 (void *)pkt1->pkt.init_pkt.buf_address, 
						 ((*p_count) >> 2), rdma->id); 
		 
		 /*
		  * Wait completion of the GET operation
		  */
		 while (!rdma_engine_test(rdma->id)){
			  thread_yield(); 
		 }

		 /*
		  * Send the end packet of the Rendez-vous protocol
		  */
		 pkt2 = (rdma_pkt_t *)kernel_malloc(pkt_size, false);
		 pkt2->pkt_type = RDMA_PKT_RDV_END;
		 pkt2->channel_id = rdma->id;
		 pkt2->use_float = 0; // ints only
		 pkt2->pkt.end_pkt.ack = 1; 
		 rdma_engine_send_pkt(rdma->tgt_rank, rdma->tgt_cpu_id, pkt2);

		 /*
		  * Unregister the buffer and free the received init pkt which was
		  * malloced in the dnp_poll function
		  */
		 rdma_engine_unregister_buffer(rdma ->id);
		 kernel_free(pkt1);

	}else{ /* EAGER */

		 write_index = 0, header_nwords = (sizeof(rdma_pkt_t) >> 2);
		 dest_addr = (uint32_t *)destination; 

		 /*
		  * Copy the contents of the EAGER pkt payload into the 
		  * destination buffer
		  */
		 if(to_read < pkt1->pkt.eager_pkt.buf_nwords){
			  /*
			   * Copy it into two parts :
			   *   + destination buffer
			   *   + local buffer
			   */
			  uint32_t extra_words = pkt1->pkt.eager_pkt.buf_nwords - to_read;
			  uint32_t sec_part    = header_nwords + to_read;

			  /* Copy in destination buffer */
			  dna_memcpy(dest_addr, &pkt_baseaddr[header_nwords],
						 to_read << 2);
			  to_read     -= to_read; /* 0 ;-) */
			  write_index += to_read;
			  
			  /* Copy in local buffer */
			  dna_memcpy(file->lbuffer, &pkt_baseaddr[sec_part],
				     extra_words << 2);
			  file->status    = CHANNEL_RIP;
			  file->lbuf_pos  = 0;
			  file->lbuf_size = extra_words;

			  DMSG("reading less than the packet contains (%d/%d)!!!\n",
			       file->lbuf_pos, file->lbuf_size);

		 }else{
			  /*
			   * Copy all of it into :
			   *   + destination buffer
			   */
			  dna_memcpy(dest_addr, &pkt_baseaddr[header_nwords],
						 pkt1->pkt.eager_pkt.buf_nwords << 2);
			  to_read     -= pkt1->pkt.eager_pkt.buf_nwords;
			  write_index += pkt1->pkt.eager_pkt.buf_nwords;

		 }

		 /*
		  * Release the pkt which was malloced in the dnp_poll function
		  */
		 kernel_free(pkt1);

		 /*
		  * This loop treats the sending of large messages (over the DNP MTU) 
		  */
		 /*
		  *  Caution! If the reader wants more data than the receiver has to offer, this could result
		  *  in the reading of the following eager packet
		  */
		 while(to_read > 0){
			  while(!rdma_engine_get_pkt(rdma->id, &pkt1, 1)){
				   thread_yield();
			  }
			  pkt_baseaddr = (uint32_t *)pkt1; 

			  /*
			   * Copy the contents of the eager packet to the destination
			   * buffer with correct offset
			   */
			  dna_memcpy(&dest_addr[write_index], &pkt_baseaddr[header_nwords],
						 pkt1->pkt.eager_pkt.buf_nwords << 2);
			  to_read     -= pkt1->pkt.eager_pkt.buf_nwords;
			  write_index += pkt1->pkt.eager_pkt.buf_nwords;

			  /*
			   * Release the pkt which was malloced in the dnp_poll function
			   */
			  kernel_free(pkt1);
		 }
	}
	
	return DNA_OK;
}



