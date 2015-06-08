#ifndef __DNP_TYPES_H__
#define __DNP_TYPES_H__



/*
 * Registers declaration ...
 */
#include <Private/dnp_registers.h>
#include <Private/dnp_rdma_pkt.h>

#if 0
/* x-y-z coordinates of the local tile */
#define DNP_REG_3DCOORD_ME  (DNP_AHB_OFFSET + 0x6c)
/* minimum coordinate inside the chip */
#define DNP_REG_MYCHIP_3DCOORD  (DNP_AHB_OFFSET + 0x70)
/* Dimension of the topology */
#define DNP_REG_MYCHIP_SIZE  (DNP_AHB_OFFSET + 0x74)
/* Dimensions of the lattice */
#define DNP_REG_LATTICE_SIZE  (DNP_AHB_OFFSET + 0x78)
/* First bit set to 1 if LUT MISS */
#define REG_DNP_FLAG_LUT_MISS  (DNP_AHB_OFFSET + 0xe8)
#define MSK_DNP_FLAG_LUT_MISS  1
/* First bit set to 1 if RB Full */
#define REG_DNP_FLAG_RB_FULL  (DNP_AHB_OFFSET + 0xec)
#define MSK_DNP_FLAG_RB_FULL  1
/* First bit set -> RB restart */
#define REG_DNP_FLAG_RB_RESTART  (DNP_AHB_OFFSET + 0xf0)
#define MSK_DNP_FLAG_RB_RESTART  1


/* RB start address */
#define REG_DNP_RBSA  (DNP_BASE_ADDR + 0x110)
#define REG_DNP_RBEA  (DNP_BASE_ADDR + 0x114)
#define REG_DNP_RBNR  (DNP_BASE_ADDR + 0x118)
#define REG_DNP_RBNW  (DNP_BASE_ADDR + 0x11c)

#endif
/*
 * END Registers declaration ...
 */

#define dnp_reg_write(idx, val) \
	 cpu_write(UINT32, DNP_BASE_ADDR + idx*sizeof(uint32_t), val)

#define dnp_reg_read(idx, ptr) \
	 cpu_read(UINT32, DNP_BASE_ADDR + idx*sizeof(uint32_t), ptr)


enum {
	 NOT_INIT_NET_FOOTER_VAL=0xfef0fef0,
	 COMPLETION_WORD_PAD_VAL=0xcec0cec0,
	 COMPLETION_DUMMY_FLAGS_VAL=0xcec1cec1,
	 COMPLETION_DUMMY_MAGIC_VAL=0xc1c1c1c1,
	 RDMA_DUMMY_ADDR=0xf1faf1fa,
	 RDMA_DUMMY_MAGIC=0xfedececa,
        // used as magic word in RDMA vaddr_dst
	 DNP_STREAMING_BUFFER_MAGIC=0xffffffff,
	 FLAGS_COMPLETION_ERROR=0,
	 MAGIC_COMPLETION_ERROR=0 
};

/*****************************************
 * LUT type
 *****************************************/
typedef struct dnp_lut_entry dnp_lut_entry_t;
struct dnp_lut_entry {
        int8_t  *start_addr;
        int8_t  *end_addr;
        uint32_t magicw;
        uint32_t flags;

#define MSK_LUT_FLAG_VALID              1
#define MSK_LUT_FLAG_STREAMING_BUFFER (1<<1) 
#define MSK_LUT_FLAG_GEN_PUT          (1<<2) 
#define MSK_LUT_FLAG_GEN_GETREQ       (1<<3)
#define MSK_LUT_FLAG_GEN_GETRESP      (1<<4)

};

/*******************************************
 * Completion event
 ******************************************/

typedef enum DNP_PKT_HDR_RDMA_CMD DNP_PKT_HDR_RDMA_CMD_T;
typedef struct dnp_net_hdr dnp_net_hdr_t;
typedef struct dnp_net_ftr dnp_net_ftr_t;
typedef struct dnp_rdma_hdr dnp_rdma_hdr_t;
typedef struct cq_entry cq_entry_t;

enum DNP_PKT_HDR_RDMA_CMD {
	 DNP_PKT_HDR_RDMA_NONE        = 0,
	 DNP_PKT_HDR_RDMA_PUT         = 1,
	 DNP_PKT_HDR_RDMA_GET_REQ     = 2,
	 DNP_PKT_HDR_RDMA_GET_RESP    = 3,
	 DNP_PKT_HDR_RDMA_NCMDS
};


struct dnp_net_hdr {

  //--------------
  // 32 bit word 0

  unsigned flags:2;


   /** This field contains information on address correctness
   * If this bit is 1 the packet must be discarded, and a completion has to be wrote for it
   */
  unsigned pkt_error:1;


   /** This field contains the step NoC id used in 4D.
   *
   */

  unsigned noc_id:4;

  /** This field contains the destination master interface.
   * It can be that M1 only access some kind of devices and M2 others.
   */
  unsigned dest_dev:1;
   
   /** This fields contains the destination DNP (Torus Coordinate) address.
   */

  unsigned dest_x:6;
  unsigned dest_y:6;  
  unsigned dest_z:6;
/*   unsigned dest_t:4; */
  
  /** This field contains the Virtual Channel to use during pkt traveling
   * Virtual Channels are used to avoid deadlocks.
   */
  unsigned vchan:4;
  
/*   #define DNP_PKT_HDR_UNICAST       0 */
/*   #define DNP_PKT_HDR_BRCAST        1 */

  /** This field contains the broadcast pkt flag
   * If it is a broadcast pkt, it is cloned, one copy keeps on
   * traveling, the other is placed on local receive queue.
   */
  unsigned brcast:1;
   
#define DNP_PKT_HDR_UNICAST       0
#define DNP_PKT_HDR_BRCAST        1

  /** This field contains the broadcast pkt flag
   * If it is a broadcast pkt, it is cloned, one copy keeps on
   * traveling, the other is placed on local receive queue.
   */


  /** This value flags a statically routed pkt.
   * Pkts are routed according to a simple
   * lexicographic scheme, e.g. first exaust along Z, then Y, the X
   */
  #define DNP_PKT_HDR_ROUTING_STATIC  0

  /** This value flags a dynamically routed. 
   * Pkts are routed according to a complex logic,
   * taking care of congestion, link state, ...
   */
  #define DNP_PKT_HDR_ROUTING_DYNAMIC  1

  /** This field contains the pkt routing type
   * It can be DNP_PKT_HDR_ROUTING_STATIC or DNP_PKT_HDR_ROUTING_DYNAMIC
   */
  unsigned routing:1;


  //--------------
  // 32 bit word 1

  #define DNP_PKT_HDR_PROTO_LOOPBACK 0
  #define DNP_PKT_HDR_PROTO_RDMA     1
  #define DNP_PKT_HDR_PROTO_MPI      2
  #define DNP_PKT_HDR_PROTO_LOFAMO         4

  /** This field contains protocol dependent flags.
   * It marks the type of protocol packet which is encapsulated inside
   * this packet.
   * LOOPBACK is intended for a simplified test of DNP_CMD_MEMCOPY.
   * MPI proto is not necessary if MPI is implemented on top of RDMA.
   */
  /* Protocol LOFAMO is used to mark packets containing fault */
  /*awareness diagnostic messages destinated to the DNP fault manager */
  unsigned proto:3;

  /** This fields contains the source DNP device (M0 or M1).
   */
  unsigned src_dev:1;
  
  /** This fields contains the source DNP (Torus Coordinate) address.
   */

  unsigned src_x:6;
  unsigned src_y:6;
  unsigned src_z:6;
/*   unsigned src_t:4; */
  /** This field contains the payload length, not including header and footer
   * The max pkt size is 1020 bytes
   */  
  #define DNP_PKT_MAX_PAYLOAD_SIZE 1020
  unsigned len:10;
};

struct dnp_net_ftr {

  //--------------
  // 32 bit word 0
 
  /** This field contains the CRC of the pkt.
   * It is computed using the header,the payload and word0 of the footer.
   * mask: 0x0000ffff
   */
  unsigned crc:16;

  /** This field flags the pkt as corrupted due to crc error.
   * RX state machines set err to 1 in case of crc error, during RX
   * phase.
   * mask: 0x00010000
   */
  unsigned err:1;

  /** These are spare bits.
   * mask: 0x00FE0000
   */
  unsigned spare:7;

  /** This field contains the pkt hop count.
   * Every time the pkt is forwarded from one DNP to the next, this
   * field is incremented.
   * mask: 0xFF000000
   */
  unsigned nhops:8;   
};

struct dnp_rdma_hdr {
  
  //--------------
  // 32 bit word 0
  /** This fields contains the DNP (Torus Coordinate) address of the target GET_RESP pkt.
   */

  unsigned tgt_x:6;
  unsigned tgt_y:6;
  unsigned tgt_z:6;
/*   unsigned tgt_t:4; */

  /** This field contains the length of RDMA operation, not including header and footer.
   * The max length is 1024 bytes but (may be) is limited by max pkt size.
   */
  unsigned len:10;


  /** 
   */
  unsigned tgt_dev:1;

  /** This field contains the RDMA command. see enum DNP_PKT_HDR_RDMA_CMD
   */
  unsigned cmd:3;

  //--------------
  // 32 bit word 1

  /** This field contains the virtual address of the RDMA operation.
   *
   * vaddr is a byte virtual address, to be used by the target master
   * interface to generate a DMA. Unaligned operations are taken care
   * by user defined logic.
   *
   * REMOVED: It is split into 2 vars to stop GCC aligning it to
   * 64bit, leaving a 32bit hole.
   *
   * CHANGED: back to 32bit
   */
  uint32_t vaddr_src;

  //--------------
  // 32 bit word 2

  uint32_t vaddr_dst;

};


struct cq_entry {
	 struct dnp_net_hdr  net_hdr;  // both buffer & RDMA events
	 struct dnp_rdma_hdr rdma_hdr; // both buffer & RDMA events
	 struct dnp_net_ftr  net_ftr;  // both buffer & RDMA events
	 uint32_t   flags;    // only buffer events
	 uint32_t   magic;    // both buffer & RDMA events
};

#define DNP_COMPLETION_NWORDS (sizeof(cq_entry_t)/4)

#define DNP_MAX_PAYLOAD_SIZE 1020
#define DNP_MAX_PACKET_SIZE (DNP_MAX_PAYLOAD_SIZE + sizeof(rdma_pkt_t))




typedef struct engine_cmd engine_cmd_t;
struct engine_cmd {

	unsigned pad1     :13;
	unsigned dest_x  	:6;
	unsigned dest_y  	:6;
	unsigned dest_z  	:6;
/* 	unsigned dest_t         :4; */
	/*This field MUST contains the 3D coordinate of the destination DNP*/
	unsigned dest_dev  :1;
	/*This field MUST contains one of the previous constant: e.g destdev may be equal to 1, then Master M1 is  selected to perform the  command*/

#define ENG_CMD_DEST_X_BIT   13UL
#define ENG_CMD_DEST_Y_BIT   19UL
#define ENG_CMD_DEST_Z_BIT   25UL
/* #define ENG_CMD_DEST_T_BIT   27UL */
#define ENG_CMD_DEST_D_BIT   31UL

#define ENG_CMD_DEST_X_LEN   6UL
#define ENG_CMD_DEST_Y_LEN   6UL
#define ENG_CMD_DEST_Z_LEN   6UL
/* #define ENG_CMD_DEST_T_LEN   4UL */
#define ENG_CMD_DEST_D_LEN   1UL

#define ENG_CMD_DEST_X_MSK   ((1<<ENG_CMD_DEST_X_LEN-1)<<ENG_CMD_DEST_X_BIT)
#define ENG_CMD_DEST_Y_MSK   ((1<<ENG_CMD_DEST_Y_LEN-1)<<ENG_CMD_DEST_Y_BIT)
#define ENG_CMD_DEST_Z_MSK   ((1<<ENG_CMD_DEST_Z_LEN-1)<<ENG_CMD_DEST_Z_BIT)
/* #define ENG_CMD_DEST_T_MSK   ((1<<ENG_CMD_DEST_T_LEN-1)<<ENG_CMD_DEST_T_BIT) */
#define ENG_CMD_DEST_D_MSK   ((1<<ENG_CMD_DEST_D_LEN-1)<<ENG_CMD_DEST_D_BIT)


#define ENG_CMD_WORD0(D_X, D_Y, D_Z, D_T, D_D) (			\
  (((D_X) << ENG_CMD_DEST_X_BIT) & ENG_CMD_DEST_X_MSK) |	\
  (((D_Y) << ENG_CMD_DEST_Y_BIT) & ENG_CMD_DEST_Y_MSK) |	\
  (((D_Z) << ENG_CMD_DEST_Z_BIT) & ENG_CMD_DEST_Z_MSK) |	\
/*   (((D_T) << ENG_CMD_DEST_T_BIT) & ENG_CMD_DEST_T_MSK) | */	\
  (((D_D) << ENG_CMD_DEST_D_BIT) & ENG_CMD_DEST_D_MSK) )


/************************1ST WORD**************************************************/  
	
	uint32_t len; //packet Length

#define ENG_CMD_WORD1(L) (L)

/************************2ND WORD**************************************************/  

	unsigned pad2     :13;//it completes the 32 bits of the word

	unsigned src_x  	:6;
	unsigned src_y  	:6;
	unsigned src_z  	:6;
/* 	unsigned src_t          :4; */

#define DNP_DEV_M0_IF 0
#define DNP_DEV_M1_IF 1

	unsigned src_dev   :1; 
	/*This field MUST contains one of the previous constant: e.g srcdev may be equal to 0, then M0 is  selected*/

/************************3RD WORD**************************************************/  

	unsigned pad3     :15; //it completes the 32 bits of the word

	unsigned broadcast:1; // indicates if the command has to be broadcasted to the network
	// completion (aka End of Operation):
	//
	//   properly alert the user application when the current operations
	//   completes
	//
	//   alerting done via:
	//     - irq rising (user app waits on irq)
	//     - writing a Completion Queue entry (user app does polling wait on the CQ)

#define COMP_NONE         0
#define COMP_IRQ          (1<<0)
#define COMP_CQ           (1<<1) // on the last (or unique) pkt, please generate a CQ event
#define COMP_CQ_FRAG      (1<<2) // if it expands in a multi-pkt send, generate CQ event for each pkt
#define COMP_LAST_PKT     (1<<3) // unused
#define COMP_UNUSED       (1<<4)
#define COMP_RDMA_EVENT   (1<<5)

	unsigned comp     :8;

	// desc:
	//   dummy command, do nothing or rise an exception
#define DNP_CMD_NONE             0U
	// desc:
	//   an intra-tile data copy from (srcdev:srcaddr) to
	//   (destdev:destaddr)
	//
	// constraints:
	//   srcdnp==destdnp==localdnp
#define DNP_CMD_MEMCOPY          1U
	// desc:
	//   a possibly-remote systolic send-recv.
	//   - local data (srcdev:srcaddr:len) are sent to remote dnp (destdnp)
	//   - remote incoming data from (srcdnp) are moved to local device
	//   (destdev:destaddr)
	//
	// constraints:
	//   len <= max pkt size
#define DNP_CMD_SYSTOLICSNDRCV   2U
	// desc:
	//   generate 1 pkt with data read from local tile
	//   read local data from (srcdev:srcaddr:len)
	//   generate a pkt with destination (destdnp:destdev:destaddr)
	//
	// constraints: 
	//   len <= max pkt size
	//   srcdnp==localdnp
	//
	// comments:
	//   leave alone the case for multi pkts RDMA ops
#define DNP_CMD_RDMA_PUT         3U
	// desc:
	//   generate 1 special pkt, send it to (srcdnp) soliciting it to
	//   generate a RDMA_PUT with data taken from (srcdev:srcaddr:len)
	//   addressed to (destdnp:destdev:destaddr)
	//
	// constraints:
	//   len <= max pkt size
	//
	// comments:
	//   leave alone the case for multi pkts RDMA ops
#define DNP_CMD_RDMA_GET         4U
	
#define DNP_NUM_CMDS (DNP_CMD_RDMA_GET+1)

	unsigned cmd      :8; // This field MUST contains one of the previous constant: e.g cmd may be uquals to 4, then is a DNP_CMD_RDMA_GET
	
/************************4ST WORD**************************************************/
	
	uint32_t srcaddr;
	
/************************5TH WORD**************************************************/  

	uint32_t destaddr;

/************************6TH WORD**************************************************/  

	// if comp&(COMP_CQ|COMP_CQ_FRAG) != 0: this is the magic word written into the Completion Queue
	uint32_t compmagic;

/************************7TH WORD**************************************************/  

};


#endif /* __DNP_TYPES_H__ */
