// This file deals with the completion queue of the DNP. event_handler
// reads all available fields in ring buffer, then fills the
// corresponding event mailbox. 

#include <MemoryManager/MemoryManager.h>
#include <Processor/Processor.h>
#include <Private/Driver.h>
#include <Private/Dnp.h>

#ifdef DNP_RDMA_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dnp_rdma:event"
#include <Private/debug.h>

#define DNP_POISON_WORD    0xBAADFACE

int8_t **streaming_buffers;

/*  dnp_event_init
 *      Description: initializes  the DNP RB, and  the mailboxes
 */
void dnp_event_queue_init()
{
	 uint32_t id = 0;
	 int i = 0;
#if 0
	 uint32_t flag_rb_full;
#endif
	 uint32_t status;

	 dnp_mailbox_init(DNP_CHANNELS_NDEV);

	 /* allocate mailboxes to hold DNP completion queue events */
	 for (id=0; id < DNP_CHANNELS_NDEV; id++)
	   dnp_mailbox_allocate(id); 
	 
	 /* Poison event queue */
	 for(i = 0; i < DNP_SETTINGS.evq_size/4; i++){
		  DNP_SETTINGS.evq_buffer[i] = DNP_POISON_WORD;
	 }

#if 1 /* Simulation model */

	 dnp_reg_read(RDMACFG1_IDX, status);

	 /*
	  * Reset device first
	  */
	 dnp_reg_write(RDMACFG1_IDX, (status & ~RDMA_RESET_M));
	 dnp_reg_write(RDMACFG1_IDX, (status | RDMA_RESET_M));

	 /* Register event queue */
	 dnp_reg_write(RDMACFG2_IDX, DNP_SETTINGS.evq_buffer); /* set RBSA */
	 dnp_reg_write(RDMACFG3_IDX, ((uint32_t)(DNP_SETTINGS.evq_buffer) + DNP_SETTINGS.evq_size - 1)); /* set RBEA */
	 dnp_reg_write(RDMACFG4_IDX, DNP_SETTINGS.evq_buffer); /* set RBNR */
	 /* dnp_reg_write(RDMACFG2_IDX REG_DNP_RBNW,  DNP_SETTINGS.evq_buffer); */

#if 0
	 dnp_reg_read(RDMAEXC_IDX, flag_rb_full);
	 flag_rb_full = flag_rb_full & ~RDMA_RB_FULL_M; //clear flag
	 dnp_reg_write(RDMAEXC_IDX, flag_rb_full);
#endif
	 
	 /*
	  * Then restart device
	  */
	 dnp_reg_write(RDMACFG1_IDX, (status & ~RB_RESTART_M));
	 dnp_reg_write(RDMACFG1_IDX, (status | RB_RESTART_M)); 

	 /*
	  * Activate divers useful parts
	  */
	 status |= (RB_RESTART_M | RDMA_ENABLE_M | LUT_ENABLE_M | RB_ENABLE_M);
	 dnp_reg_write(RDMACFG1_IDX, status);

#else /* APENET model */
	 /*
	  * We are only usin EQ0 for now
	  */

	 /* Disable */
	 dnp_writel(dnp_dev, APEDEV_REG_EQ0_COMMAND, 0x0);
	
	 /* Set EQ Buffer address */
	 dnp_writel(dnp_dev, APEDEV_REG_EQ0_SADDR_LO, (uint32_t)DNP_SETTINGS.evq_buffer);
	 dnp_writel(dnp_dev, APEDEV_REG_EQ0_SADDR_HI, 0x0);

	 /* Set EQ Buffer size */
	 dnp_writel(dnp_dev, APEDEV_REG_EQ0_NEVENTS, DNP_SETTINGS.evq_size);
	 /* Set read pointer to NR==NW */
	 dnp_writel(dnp_dev, APEDEV_REG_EQ0_NR,      0x0);
	 /* Remove mask from BAD_NR and BAD_RANGE */
	 dnp_writel(dnp_dev, APEDEV_REG_EQ0_MASKSTATUS, EQ_MASKSTATUS_MASK_FULL);
	 /* Set enabled and restart */
	 dnp_writel(dnp_dev, APEDEV_REG_EQ0_COMMAND, EQ_COMMAND_ENABLED|EQ_COMMAND_RESTART);

	 blocking_nsleep(100);
	 /* Leave enabled */
	 dnp_writel(dnp_dev, APEDEV_REG_EQ0_COMMAND, EQ_COMMAND_ENABLED);


#if 1
	 {
		  uint32_t addrlo = 0;
		  uint32_t addrhi = 0;
		  uint32_t ne     = 0;
		  uint32_t rbnw   = 0;
		  uint32_t rbnr   = 0;
		  uint32_t nrdy   = 0;
		  uint32_t msksts = 0;
		  uint32_t cmd    = 0;

		  addrlo = dnp_readl(dnp_dev, APEDEV_REG_EQ0_SADDR_LO  );
		  addrhi = dnp_readl(dnp_dev, APEDEV_REG_EQ0_SADDR_HI  );
		  ne     = dnp_readl(dnp_dev, APEDEV_REG_EQ0_NEVENTS   );
		  rbnr   = dnp_readl(dnp_dev, APEDEV_REG_EQ0_NR        );
		  rbnw   = dnp_readl(dnp_dev, APEDEV_REG_EQ0_NW        );
		  nrdy   = dnp_readl(dnp_dev, APEDEV_REG_EQ0_RDYEVENTS );
		  msksts = dnp_readl(dnp_dev, APEDEV_REG_EQ0_MASKSTATUS);
		  cmd    = dnp_readl(dnp_dev, APEDEV_REG_EQ0_COMMAND   );

		  DMSG("EQ0: addr=0x%lx size=%dbytes\n",
			   DNP_SETTINGS.evq_buffer, DNP_SETTINGS.evq_size);
		  DMSG("ADDR HI=0x%x LO=0x%x NEVENTS=0x%x\n", 
			   addrhi, addrlo, ne);
		  DMSG("RBNR=0x%x RBNW=0x%x RDYEVENTS=0x%x "
			   "MASKSTATUS=0x%x COMMAND=0x%x\n", 
			   rbnr, rbnw, nrdy, msksts, cmd);

	 }
#endif

#endif /* Simulation model */



	 DMSG("Activating the DNP with:\n");
	 DMSG(" + event queue: 0x%x - 0x%x\n",
		  DNP_SETTINGS.evq_buffer,
		  ((uint32_t)(DNP_SETTINGS.evq_buffer) + DNP_SETTINGS.evq_size - 1));

	 streaming_buffers = kernel_malloc(DNP_SETTINGS.nstreaming_buffer*sizeof(int8_t *), false); 

	 for(i = 0; i < DNP_SETTINGS.nstreaming_buffer; i++){
	   streaming_buffers[i] = kernel_malloc(DNP_SETTINGS.streaming_size, false);

	   DMSG(" + streaming buffer: 0x%x (0x%x)\n",
		streaming_buffers[i],
		DNP_SETTINGS . streaming_size);
	   // Declare streaming buffer, to receive PUT requests
	   rdma_engine_register_streaming_buffer(streaming_buffers[i],
						 DNP_SETTINGS.streaming_size,
						 0xDEADBEEF); 

	 }

	return ;
}

/*  dnp_event_open
 *      Description:  
 */
void
dnp_event_open(uint32_t channel_id)
{
}


/*  dnp_event_get_request
 *      Description: gets first available event in mailbox 
 */
dnp_status_t
dnp_event_request(uint32_t channel_id, uint32_t  mb_dir, dnp_event_t *event, int blocking)
{
  dnp_status_t stat = DNP_SUCCESS;
  stat = dnp_mailbox_pop_mail(channel_id, event, mb_dir, blocking);
  
  if(stat == DNP_SUCCESS){
  
    DMSG("request: event found in the mailbox %d in %s direction\n",
	 (int)channel_id, mb_dir? "output":"input");
    DMSG("request: len: %d\n", (int)event->len);
  }
  return stat;
}

/*  dnp_poison_compentry(uint32_t *);
 *  poisons a compqueue entry . Useful when for AHB trafic reasons, the RB next write
 *  is set before completion word is available
 */
void dnp_poison_compentry(uint32_t *compentry)
{
    for (int i=0; i<DNP_COMPLETION_NWORDS; i++)
    {
#if 0
        cpu_write(UINT32, &compentry[i], DNP_POISON_WORD);
#else
	compentry[i] = DNP_POISON_WORD;
#endif
    }
}

static inline void
dnp_poll_print_cqe(cq_entry_t *cqe){

#if defined(DEBUG) || defined(__STRICT_ANSI_C__)
	 dnp_net_hdr_t  *net_hdr  = &(cqe->net_hdr);
	 dnp_rdma_hdr_t *rdma_hdr = &(cqe->rdma_hdr);
	 dnp_net_ftr_t  *net_ftr  = &(cqe->net_ftr);
#endif

	 DMSG("== **** CQ Entry **** ==\n");

	 DMSG(" **** net_hdr ****\n");
	 DMSG("dstx: 0x%x, dsty: 0x%x, dstz: 0x%x, "
		  "dstdev: 0x%x, flags: 0x%x, pkt_error: 0x%x, noc_id: 0x%x\n",
		  net_hdr->dest_x, net_hdr->dest_y, net_hdr->dest_z,
		  net_hdr->dest_dev, net_hdr->flags, net_hdr->pkt_error,
		  net_hdr->noc_id);
	 DMSG("srcx: 0x%x, srcy: 0x%x, srcz: 0x%x, srcdev: 0x%x, "
		  "len: 0x%x\n",
		  net_hdr->src_x, net_hdr->src_y, net_hdr->src_z, net_hdr->src_dev,
		  net_hdr->len);

	 DMSG(" **** rdma_hdr ****\n");
	 DMSG("tgtx: 0x%x, tgty: 0x%x, tgtz: 0x%x, len: 0x%x, tgtdev: 0x%x, "
		  "cmd: 0x%x\n",
		  rdma_hdr->tgt_x, rdma_hdr->tgt_y, rdma_hdr->tgt_z,
		  rdma_hdr->len, rdma_hdr->tgt_dev, rdma_hdr->cmd);
	 DMSG("src vaddress: 0x%x\n", rdma_hdr->vaddr_src);
	 DMSG("dst vaddress: 0x%x\n", rdma_hdr->vaddr_dst);
	 DMSG("crc: 0x%x, nhops: 0x%x, err: 0x%x\n",
		  net_ftr->crc, net_ftr->nhops, net_ftr->err);
	 DMSG("lut flags: 0x%x\n", cqe->flags);
	 DMSG("magicw: 0x%x\n", cqe->magic);
	 
	 return;
}


/*  dnp_event_poll: very important function of the driver
 *      Description: reads all CQ events, put in the events mailbox
 */
void
dnp_event_poll(void)
{
	 dnp_cart_coords_t src_coords, dest_coords, local_coords; 
	 uint32_t srcx,srcy,srcz,dstx,dsty,dstz;
	 uint32_t flag_rb_full, rb_sa, rb_ea, rb_nr, rb_nw;
	 uint8_t cmd;
	 cq_entry_t cqe;
	 dnp_event_t event;
	 dnp_mailbox_direction_t dir = MAILBOX_IN;
	 uint32_t *comp_base = (uint32_t *)&cqe;

	/*
	 * Verify in rinb buffer area of the dnp
	 */
    dnp_reg_read(RDMAEXC_IDX, flag_rb_full);
	
    dnp_reg_read(RDMACFG4_IDX, rb_nr); 
    dnp_reg_read(RDMASTS1_IDX, rb_nw);
	
    DMSG("poll: 0x%x, (NR: 0x%x, NW: 0x%x)\n", flag_rb_full, rb_nr, rb_nw);
    if ( !(flag_rb_full & RDMA_RB_FULL_M) &&
		 (rb_nr == rb_nw )                ){
        return;
    } 
	
    dnp_reg_read(RDMACFG2_IDX, rb_sa); 
    dnp_reg_read(RDMACFG3_IDX, rb_ea);

	/*
	 * read each completion queue event in system memory,
	 * put it in the good mailbox
	 */
    while ((rb_nr != rb_nw) || (flag_rb_full & RDMA_RB_FULL_M)){

		 cpu_vector_read(UINT32, &cqe, (uint8_t *)rb_nr, DNP_COMPLETION_NWORDS);

		 DMSG("poll: ***** new completion\n");
		 dnp_poll_print_cqe(&cqe);

		 /*
		  * Verify if the completion event has been received totally
		  * thans to the poison word
		  */
		 if( (comp_base[0] == DNP_POISON_WORD) || (comp_base[1] == DNP_POISON_WORD) ||
			 (comp_base[2] == DNP_POISON_WORD) || (comp_base[3] == DNP_POISON_WORD) ||
			 (comp_base[4] == DNP_POISON_WORD) || (comp_base[5] == DNP_POISON_WORD) ||
			 (comp_base[6] == DNP_POISON_WORD) || (comp_base[7] == DNP_POISON_WORD) )
			  continue;
		 
		 cmd = cqe.rdma_hdr.cmd;
		 
		 /*
		  * Make 3D coordinates -> rank conversion
		  */
		 dnp_cart_get_local_coords(&local_coords);
		 
		 dstx = cqe.net_hdr.dest_x;
		 dsty = cqe.net_hdr.dest_y;
		 dstz = cqe.net_hdr.dest_z;
		 dnp_cart_set_dim(dstx, 0, &dest_coords);
		 dnp_cart_set_dim(dsty, 1, &dest_coords);
		 dnp_cart_set_dim(dstz, 2, &dest_coords);
		 
		 srcx = cqe.net_hdr.src_x; 
		 srcy = cqe.net_hdr.src_y;
		 srcz = cqe.net_hdr.src_z;
		 dnp_cart_set_dim(srcx, 0, &src_coords);
		 dnp_cart_set_dim(srcy, 1, &src_coords);
		 dnp_cart_set_dim(srcz, 2, &src_coords);
		 
		 /*
		  * Verify if the first available event is meant for this CPU.
		  * If not, yield. 
		  * This means that the processor waits for another processor to read
		  * its completion event to continue the process
		  */
		 if(dnp_cart_coordscmp(&src_coords, &local_coords)){
			  /* ack of outgoing operation */
			  
			  if(cqe.net_hdr.src_dev != DNP_COMMON.local_cpu_id){
				   return;
			  }
		 }else{
			  if(cqe.net_hdr.src_dev != DNP_COMMON.local_cpu_id){
				   return;
			  }
		 }
		 
		 /*
		  * According to the command of the CQ entry, fill in the correct
		  * mailbox with the driver internal event representation
		  */
		 switch(cmd){
		 case DNP_PKT_HDR_RDMA_PUT:
			  DMSG("poll: src coords: (%u,%u,%u) dest coords:(%u,%u,%u)\n", 
				   srcx, srcy, srcz, dstx, dsty, dstz);
			  
			  if(dnp_cart_coordscmp(&src_coords, &local_coords)  &&
				 dnp_cart_coordscmp(&dest_coords, &local_coords) ){
				   /* Loopback PUT operation */
				   DMSG("poll: event type PUT_ACK in mailbox (LB)\n");
				   event.type = RDMA_EVENT_TYPE_PUT_ACK;
				   event.len  = cqe.rdma_hdr.len;
				   event.ptr  = cqe.rdma_hdr.vaddr_src; //source

				   if(cqe.magic == 0xDEADBEEF){
						/*
						 * The PUT operation targeted a streaming buffer
						 */
						event.type =  RDMA_EVENT_TYPE_PUT_REQ;
						event.len  = cqe.rdma_hdr.len;
						event.ptr  = cqe.rdma_hdr.vaddr_dst; // dst

						event.pkt_type = ((rdma_pkt_t *)event.ptr)->pkt_type;
						event.channel_id = ((rdma_pkt_t *)event.ptr)->channel_id;
						event.pkt = (rdma_pkt_t *)kernel_malloc(event.len, false);
						/*
						 * Copy the streaming buffer contents into the malloced area
						 * FIXME: this intermediate copy could be avoided to increase performance
						 * by having an array of streaming buffers
						 */
						dna_memcpy(event.pkt, (uint8_t *)event.ptr, event.len);
							 
						dir = MAILBOX_IN;
						 
						/*
						 * The streaming buffer has been read, it can be the target of a new PUT operation 
						 */
						rdma_engine_register_streaming_buffer((int8_t *)event.ptr, DNP_MAX_PACKET_SIZE, 0xDEADBEEF);

				   }else if(cqe.rdma_hdr.vaddr_dst == (uint32_t)DNP_STREAMING_BUFFER_MAGIC){
						/*
						 * rendez-vous packet: channel id in the packet itself
						 */

						event.pkt_type =   ((rdma_pkt_t *)event.ptr)->pkt_type;
						event.channel_id = ((rdma_pkt_t *)event.ptr)->channel_id;
						dir = MAILBOX_OUT;

				   }else{
						/*
						 * real PUT operation acknowledgement, channel id in
						 * the magic word
						 */
						event.pkt_type = RDMA_PKT_NONE;
						event.channel_id = cqe.magic;
						dir = MAILBOX_OUT;
			  
				   }
				   DMSG("poll: channel:%u len: %u ptr: 0x%x \n", 
						event.channel_id, event.len, event.ptr);


			  }else
				   if(dnp_cart_coordscmp(&src_coords, &local_coords)){
				   /*
					* Outgoing PUT operation acknowledgment
					*/
				   DMSG("poll: event type PUT_ACK in mailbox\n");
				   event.type = RDMA_EVENT_TYPE_PUT_ACK;
				   event.len  = cqe.rdma_hdr.len;
				   event.ptr  = cqe.rdma_hdr.vaddr_src; //source
						
				   if(cqe.rdma_hdr.vaddr_dst == (uint32_t)DNP_STREAMING_BUFFER_MAGIC){
						/*
						 * rendez-vous packet: channel id in the packet itself
						 */

						event.pkt_type =   ((rdma_pkt_t *)event.ptr)->pkt_type;
						event.channel_id = ((rdma_pkt_t *)event.ptr)->channel_id;
				   }else{
						/*
						 * real PUT operation acknowledgement, channel id in
						 * the magic word
						 */
						event.pkt_type = RDMA_PKT_NONE;
						event.channel_id = cqe.magic;
				   }
						
				   DMSG("poll: channel: %u len: %u ptr: 0x%x\n",
						event.channel_id, event.len, event.ptr);
				   dir = MAILBOX_OUT;
				   }
			  else if(dnp_cart_coordscmp(&dest_coords, &local_coords)){
				   /*
					* Ingoing PUT operation notice
					*/
				   event.type =  RDMA_EVENT_TYPE_PUT_REQ;
				   event.len  =  cqe.rdma_hdr.len;
				   event.ptr  =  cqe.rdma_hdr.vaddr_dst; // dst
				   
				   if(cqe.magic == 0xDEADBEEF){
						/*
						 * The PUT operation targeted a streaming buffer
						 */
						event.pkt_type = ((rdma_pkt_t *)event.ptr)->pkt_type;
						event.channel_id = ((rdma_pkt_t *)event.ptr)->channel_id;
						event.pkt = (rdma_pkt_t *)kernel_malloc(event.len, false);
						/*
						 * Copy the streaming buffer contents into the malloced area
						 * FIXME: this intermediate copy could be avoided to increase performance
						 * by having an array of streaming buffers
						 */
						dna_memcpy(event.pkt, (uint8_t *)event.ptr, event.len);
							 
						dir = MAILBOX_IN;
						 
						/*
						 * The streaming buffer has been read, it can be the target of a new PUT operation 
						 */
						rdma_engine_register_streaming_buffer((int8_t *)event.ptr, DNP_MAX_PACKET_SIZE, 
										      0xDEADBEEF);
				   }else{
						/*
						 * A normal PUT operation on a registered buffer, this is not used in the current driver
						 */
						DMSG("poll:  ERROR non supported event type PUT "
							 "in mailbox %u\n", cqe.magic);
						event.pkt_type = RDMA_PKT_NONE;
						event.channel_id = cqe.magic;
						dir = MAILBOX_IN;
							 
				   }
				   DMSG("poll: channel:%u len: %u ptr: 0x%x \n", 
						event.channel_id, event.len, event.ptr);
			  }
				   
			  break;
				   
		 case DNP_PKT_HDR_RDMA_GET_REQ:
			  /*
			   * Outgoing GET operation acknowledgment
			   * This comes at the very end of a GET operation on reader side, 
			   *  When the payload exceeds the maximum packet size, it is 
			   * triggered once the very last  packet has been received
			   */
			  DMSG("poll: event type GET_REQ in mailbox\n");
			  event.type = RDMA_EVENT_TYPE_GET_ACK;
			  event.len  = cqe.rdma_hdr.len;
			  event.ptr  = cqe.rdma_hdr.vaddr_src; //source
			  event.channel_id  = cqe.magic; 
			  dir = MAILBOX_OUT;
			  break;
				   
		 case DNP_PKT_HDR_RDMA_GET_RESP:
			  /*
			   * In coming or outgoing GET acknowledgement. These events come
			   * after each emission/reception of GET operation packets
			   */
			  DMSG("poll: GET RESP  src coords: (%d,%d,%d) dest coords:(%d,%d,%d)\n", 
				   srcx, srcy, srcz, dstx, dsty, dstz);
			  if (dnp_cart_coordscmp(&src_coords, &local_coords)){
				   /*
					* Outgoing get ack on sender side
					*/
				   DMSG("poll: event type GET_ACK in mailbox\n");
				   event.type = RDMA_EVENT_TYPE_GET_ACK;
				   event.len  = cqe.rdma_hdr.len;
				   event.ptr  = cqe.rdma_hdr.vaddr_src; //source
				   event.channel_id  = cqe.magic; 
				   dir = MAILBOX_OUT;
			  }else if (dnp_cart_coordscmp(&dest_coords, &local_coords)){
				   /*
					* Outgoing get ack on receiver side
					*/
				   DMSG("poll: event type GET_RESP in mailbox %u\n",
						cqe.magic);
				   event.type =  RDMA_EVENT_TYPE_GET_RSP;
				   event.len  =  cqe.rdma_hdr.len;
				   event.ptr  =  cqe.rdma_hdr.vaddr_dst;
				   event.channel_id  = cqe.magic; 
				   dir = MAILBOX_IN;
			  }
			  DMSG("poll: GET_RESP channel:%u len: %u ptr: 0x%x dir: %d \n",
				   event.channel_id, event.len, event.ptr, (int)dir);
			  break;
		 default:
			  DMSG("poll: Bad RDMA type received\n");
		 }
			  
		 /*
		  * So far , we have a DNP event, we can push it into the correct
		  * channel mailbox
		  */
		 dnp_mailbox_push_mail(event.channel_id , &event, dir);
			  
		 /*
		  * poison treated completion entry
		  */
		 dnp_poison_compentry((uint32_t *)rb_nr);
			
		 /*
		  * update rb pointer and flag
		  */
		 rb_nr = rb_sa + (rb_nr - rb_sa + DNP_COMPLETION_NWORDS*4 )  %  (rb_ea - rb_sa + 1); /* FIXME FIXME */
		 flag_rb_full = flag_rb_full & ~RDMA_RB_FULL_M; //clear flag

		 /*
		  * update dnp rb registers
		  */

		 dnp_reg_write(RDMAEXC_IDX, flag_rb_full);
		 dnp_reg_write(RDMACFG4_IDX, rb_nr); 
		 
		 if(dnp_mailbox_is_full(event.channel_id, dir)) 
		 {
			  DMSG("poll: warning, MB %u  full\n", event.channel_id);
			  break;  
		 }
	}
	
}
	
