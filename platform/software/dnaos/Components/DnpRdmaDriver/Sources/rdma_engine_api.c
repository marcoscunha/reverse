#include <MemoryManager/MemoryManager.h>
#include <Private/Dnp.h>
#include <Private/Driver.h>
#include <Private/status.h>
#include <Processor/Processor.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma"
#include <Private/debug.h>


/*
 * Send a buffer description for registration
 */
void
dnp_send_lut_entry(dnp_lut_entry_t *lutentry, uint32_t lut_id){

	 interrupt_status_t  it_status = cpu_trap_mask_and_backup();
	 dnp_lut_entry_t *ptr = (dnp_lut_entry_t *)(DNP_BASE_ADDR +  DNP_OFFSET_LUT_START + lut_id * sizeof(dnp_lut_entry_t) ) ;
	 cpu_vector_write(UINT32, ptr, lutentry, sizeof(dnp_lut_entry_t) / 4);

	 cpu_trap_restore(it_status);
}

/*
 * Read a registered buffer description
 */
void
dnp_read_lut_entry(dnp_lut_entry_t *lutentry, uint32_t lut_id){

	 interrupt_status_t  it_status = cpu_trap_mask_and_backup();
	 dnp_lut_entry_t *ptr = (dnp_lut_entry_t *)(DNP_BASE_ADDR +  DNP_OFFSET_LUT_START + lut_id * sizeof(dnp_lut_entry_t) ) ;
	 cpu_vector_read(UINT32, lutentry, ptr, sizeof(dnp_lut_entry_t) / 4);

	 cpu_trap_restore(it_status);
}

/*
 * Send a command to the slave interface of the dnp
 */
void
dnp_send_cmd(engine_cmd_t *cmd)
{
	 interrupt_status_t  it_status = cpu_trap_mask_and_backup();
	 /* Mask interrupts to avoid interleaving the filling of CMD fifo */
	 engine_cmd_t *ptr = (engine_cmd_t *)(DNP_BASE_ADDR + DNP_OFFSET_CMD_FIFO_START) ;
	 cpu_vector_write(UINT32, ptr, cmd, sizeof(engine_cmd_t) / 4);

	 cpu_trap_restore(it_status);
}


/*
 * Initialize hardware-dependant part of the driver
 */
void
rdma_engine_init(void){

	 /*
	  * Initialize topological settings in DNP
	  */
	 dnp_topo_init();
	 /*
	  * Initialise events
	  */
	 dnp_event_queue_init();

}

/*
 * Register a streaming buffer for PUT operations
 */
int32_t
rdma_engine_register_streaming_buffer(int8_t *startptr, int32_t len,
									  uint32_t magic){

#if 1 /* Simulaion model */

	 uint32_t i;
	 dnp_lut_entry_t lut;

	 /*
	  * search for a free slot in luts
	  */
	 for(i = 0; i < DNP_RDMA_LUT_NENTRIES; i++){
		  dnp_read_lut_entry(&lut, i);
		  if(!(lut.flags & DNP_RDMA_LUT_FLAG_VALID))
			   break;
	 }
	 
	 if(i == DNP_RDMA_LUT_NENTRIES)
		  return -1;
    
	   dnp_lut_entry_t lutentry;

	   /* Create LUT entry in DNP */
	   lutentry.start_addr = startptr;
	   lutentry.end_addr   = startptr + len;
	   lutentry.magicw     = magic;
	   lutentry.flags      = DNP_RDMA_LUT_FLAG_VALID  |
			DNP_RDMA_LUT_FLAG_STREAMING | DNP_RDMA_LUT_FLAG_GEN_PUT;

	   dnp_send_lut_entry(&lutentry, i);
	   return 0;

#else /* APENET model */

	 dnp_micro_cmd_reg_buf_t  cmd = EMPTY_CMD_REG_BUF;
	 
	 cmd.hdr.portid   = 0; /* We only use 0 now */
	 cmd.hdr.op       = DNP_MICRO_OP_REG_BUF;

	 cmd.virt_addr = (uint64_t)(uint32_t)startptr;
	 cmd.len       = len;
	 cmd.flags     = DNP_STREAMING_BUFFER;
	 cmd.magic     = magic;
	 
	 dnp_micro_send_cmd(&cmd);
	 return 0;
#endif
}


/*
 * Open a RDMA channel
 */
void
rdma_engine_open(uint32_t channel_id){
    dnp_event_open(channel_id);
}

/*
 * Register a regular  src or target buffer zone for a GET operation
 */
int32_t
rdma_engine_register_buffer(int8_t *startptr, int32_t len,
							uint32_t magic)
{
    uint32_t i;
    dnp_lut_entry_t lut;

	/*
	 * search for a free slot in luts
	 */
    for(i = 0; i < DNP_RDMA_LUT_NENTRIES; i++){
        dnp_read_lut_entry(&lut, i);
        if(!(lut.flags & DNP_RDMA_LUT_FLAG_VALID))
            break;
    }

	if(i == DNP_RDMA_LUT_NENTRIES)
        return -1;
    
    dnp_lut_entry_t lutentry;

    /*
	 * Create LUT entry in DNP
	 */
    lutentry.start_addr = startptr;
    lutentry.end_addr   = startptr + len;
    lutentry.magicw = magic ; 
    lutentry.flags  = DNP_RDMA_LUT_FLAG_VALID | DNP_RDMA_LUT_FLAG_GEN_PUT; 

    dnp_send_lut_entry(&lutentry, i);
    return 0;
}

/*
 * Unregister regular buffers
 */
int32_t
rdma_engine_unregister_buffer(uint32_t magic)
{
    uint32_t i;
    dnp_lut_entry_t  lut,lutentry = {0,0,0,0};

	/*
	 * search for channel id in magic word of luts
	 */
    for ( i = 0; i < DNP_RDMA_LUT_NENTRIES; i++ )
    {
        dnp_read_lut_entry(&lut, i);
        if (lut.magicw == magic)
            break;
    }
    if (i == DNP_RDMA_LUT_NENTRIES)
        return -1;

    dnp_send_lut_entry(&lutentry, i);
    
    return 0;
}


/*
 * Get a rdma pkt, this method is non-blocking to allow the generic part
 * to yield the task
 */
int32_t
rdma_engine_get_pkt(uint32_t channel_id, rdma_pkt_t **pkt, int blocking)
{
	 dnp_event_t event;
	 int     loop_end = 0;
	 int32_t ret = RDMA_RETVAL_NONE;;

	 do {
	   dnp_status_t stat = DNP_SUCCESS;

	   stat = dnp_event_request(channel_id, MAILBOX_IN, &event, blocking);

	   if(stat == DNP_NO_MAIL){
	     if(!blocking){
	       DMSG("get_pkt: +++ got no event (non blocking)\n");
	       loop_end = 1;
         ret = RDMA_RETVAL_NONE;
	     }else{	       
	       EMSG("get_pkt: +++ got no event (while blocking)\n");
	     }
	   }else{
	     DMSG("get_pkt: +++ got an event\n");

	     switch(event.type){
	     case RDMA_EVENT_TYPE_PUT_REQ: 
	       if(event.pkt_type != RDMA_PKT_NONE){
		 DMSG("get_pkt: got one packet. %d %d %d %d\n",
		      (int)event.pkt->pkt_type,
		      (int)event.pkt->channel_id,
		      (int)event.pkt->pkt.init_pkt.buf_address,
		      (int)event.pkt->pkt.init_pkt.buf_nwords
		    );
		 
		 *pkt = event.pkt;
		 loop_end = 1;
		 ret = RDMA_RETVAL_DONE;
	       }
	       break;
	     default:
	       EMSG("get_pkt: got an unsupported ype of message here\n");
	     }
	   }
	 }while( !loop_end );

	 return ret;
}

/*
 * Check completion of GET request
 */
uint32_t
rdma_engine_test(uint32_t channel_id)
{
    dnp_event_t event;
    // not called by a separate thread
    //dnp_event_poll();

    if( (dnp_event_request(channel_id, MAILBOX_IN, &event, 0) == DNP_SUCCESS) &&
		(event.type == RDMA_EVENT_TYPE_GET_RSP)                               ){
		 DMSG("test: got a get resp.\n");
		 return 1;
    }else{
		 return 0;
	}
}

// Send a rdma pkt using a PUT targeting a streaming buffer
int32_t
rdma_engine_send_pkt(uint32_t rank, uint32_t dev, rdma_pkt_t *pkt)
{

	 dnp_cart_coords_t local_coords, dest_coords;
	 uint32_t srcx, srcy, srcz, dstx, dsty, dstz;
	 engine_cmd_t eng_cmd;
	 rdma_pkt_t *pkt_header = (rdma_pkt_t *)pkt;
	 uint32_t pkt_size = sizeof(rdma_pkt_t);
	 dnp_event_t event;

	 /*
	  * Rank -> 3D coords conversion
	  */
	 dnp_cart_get_local_coords(&local_coords);
	 dnp_cart_coords(rank, &dest_coords);
	 
	 srcx = dnp_cart_get_dim((uint32_t)0, &local_coords);
	 srcy = dnp_cart_get_dim((uint32_t)1, &local_coords);
	 srcz = dnp_cart_get_dim((uint32_t)2, &local_coords);
	 dstx = dnp_cart_get_dim((uint32_t)0, &dest_coords);
	 dsty = dnp_cart_get_dim((uint32_t)1, &dest_coords);
	 dstz = dnp_cart_get_dim((uint32_t)2, &dest_coords);

	 eng_cmd.pad1     = 0;
	 eng_cmd.dest_x   =  dstx; 
	 eng_cmd.dest_y   =  dsty;
	 eng_cmd.dest_z   =  dstz; 
	 eng_cmd.dest_dev = dev;                         

	 eng_cmd.len = pkt_size;
	 if(pkt_header->pkt_type == RDMA_PKT_EAGER){
		  eng_cmd.len += (pkt_header->pkt.eager_pkt.buf_nwords << 2) ;
	 }else{
		 
	 }

	 eng_cmd.pad2     = 0;
	 eng_cmd.src_x    = srcx; 
	 eng_cmd.src_y    = srcy; 
	 eng_cmd.src_z    = srcz; 
	 eng_cmd.src_dev  = DNP_COMMON.local_cpu_id;                            

	 /*
	  * generate an event for each paquet (clustered)
	  * NB: SW takes care of not exceeding the max payload of the DNP
	  */
	 eng_cmd.pad3     = 0;
	 eng_cmd.broadcast = 0 ;
	 eng_cmd.comp      = (COMP_CQ | COMP_CQ_FRAG | COMP_RDMA_EVENT);
	 eng_cmd.cmd       = DNP_CMD_RDMA_PUT; 

	 /*
	  * write ptr of the buffer for source address
	  */
	 eng_cmd.srcaddr = (uint32_t)pkt ; 

	 /*
	  * destination address: streaming buffer
	  */
	 eng_cmd.destaddr = DNP_STREAMING_BUFFER_MAGIC; 

	 eng_cmd.compmagic = pkt_header->channel_id; //magic 

	 DMSG("Sending packet of %u bytes from (%u,%u,%u) to (%u %u %u)\n",
		  eng_cmd.len, srcx, srcy, srcz, dstx, dsty, dstz);
	 DMSG("               src: %x\n", pkt);

	 /*
	  * FIXME: Wait cmd FIFO.
	  */
	 dnp_send_cmd(&eng_cmd);

	 /*
	  * Wait HW acknowledgement
	  */
	 dnp_event_request(eng_cmd.compmagic, MAILBOX_OUT, &event, 1);
	 DMSG("+++ got an event !!!\n");
	
	 DMSG("Sending packet done\n");
	 kernel_free(pkt);

	 return 0;
}

/*
 * Initiate a GET request to peek data from remote tile
 */
int32_t
rdma_engine_get(void *dst, uint32_t dest_rank, uint32_t dest_dev, void *src,
			 uint32_t nwords, uint32_t channel_id)
{

	 uint32_t srcx, srcy, srcz, dstx, dsty, dstz;
	 dnp_event_t event;
	 dnp_cart_coords_t local_coords, dest_coords;
	 engine_cmd_t eng_cmd;

	/*
	 * Rank -> 3D coordinates conversion
	 */
    dnp_cart_get_local_coords(&local_coords);
    dnp_cart_coords(dest_rank, &dest_coords);
	
    srcx = dnp_cart_get_dim((uint32_t)0, &local_coords);
    srcy = dnp_cart_get_dim((uint32_t)1, &local_coords);
    srcz = dnp_cart_get_dim((uint32_t)2, &local_coords);
    dstx = dnp_cart_get_dim((uint32_t)0, &dest_coords);
    dsty = dnp_cart_get_dim((uint32_t)1, &dest_coords);
    dstz = dnp_cart_get_dim((uint32_t)2, &dest_coords);

    DMSG("Getting %u words from (%u %u %u) to (%u %u %u)\n",
		 nwords, dstx, dsty, dstz, srcx, srcy, srcz);

    eng_cmd.dest_x   = srcx; 
    eng_cmd.dest_y   = srcy;
    eng_cmd.dest_z   = srcz; 
    eng_cmd.dest_dev = DNP_COMMON.local_cpu_id;                         

    eng_cmd.len  = nwords << 2;

    eng_cmd.src_x   = dstx; 
    eng_cmd.src_y   = dsty; 
    eng_cmd.src_z   = dstz; 
    eng_cmd.src_dev = dest_dev; 

	/*
	 * generate an event for each paquet (clustered)
	 */
    eng_cmd.broadcast = 0 ;

	/*
	 * FIXME: DNP should be able to generate completion at the end of the GET 
	 * (not the intermediary packets)
	 */
    eng_cmd.comp      = COMP_CQ | COMP_RDMA_EVENT; // LAST_PACKET
    eng_cmd.cmd       = DNP_CMD_RDMA_GET; 

	/*
	 * write ptr of the buffer for source address
	 */
    eng_cmd.srcaddr = (uint32_t)src ; 

	/*
	 * destination address: streaming buffer
	 */
    eng_cmd.destaddr = (uint32_t)dst; 
	
    eng_cmd.compmagic = channel_id; //magic 

    dnp_send_cmd(&eng_cmd);

	/*
	 * Wait HW acknowledgement of the GET request 
	 */
    dnp_event_request(channel_id, MAILBOX_OUT, &event, 1);
    DMSG("+++ got an event !!!\n");

    DMSG("Get done\n");
    return nwords;

}
