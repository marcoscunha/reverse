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

#ifndef __DNP_TYPES_H__
#define __DNP_TYPES_H__


#include  <Private/dnp_rdma_pkt.h>


enum COORD_MASK{
    X_MASK = 0x0000003f,
    Y_MASK = 0x00000fc0,
    Z_MASK = 0x0003f000,
    X_OFFS = 0,
    Y_OFFS = 6,
    Z_OFFS = 12,
};

enum SIZE_MASK{
    X_SIZE_MASK = 0x000000ff,
    Y_SIZE_MASK = 0x0000ff00,
    Z_SIZE_MASK = 0x00ff0000,
    X_SIZE_OFFS = 0,
    Y_SIZE_OFFS = 8,
    Z_SIZE_OFFS = 16,
};


/* Registers*/

typedef uint32_t DNP_REG;

// x-y-z coordinates of the local tile
#define DNP_REG_3DCOORD_ME  (DNP_AHB_OFFSET + 0x6c)
// minimum coordinate inside the chip
#define DNP_REG_MYCHIP_3DCOORD  (DNP_AHB_OFFSET + 0x70)
// Dimension of the topology
#define DNP_REG_MYCHIP_SIZE  (DNP_AHB_OFFSET + 0x74)
// Dimensions of the lattice
#define DNP_REG_LATTICE_SIZE  (DNP_AHB_OFFSET + 0x78)
// First bit set to 1 if LUT MISS
#define REG_DNP_FLAG_LUT_MISS  (DNP_AHB_OFFSET + 0xe8)
#define MSK_DNP_FLAG_LUT_MISS  1
// First bit set to 1 if RB Full
#define REG_DNP_FLAG_RB_FULL  (DNP_AHB_OFFSET + 0xec)
#define MSK_DNP_FLAG_RB_FULL  1
// First bit set -> RB restart
#define REG_DNP_FLAG_RB_RESTART  (DNP_AHB_OFFSET + 0xf0)
#define MSK_DNP_FLAG_RB_RESTART  1

// RB start address
#define REG_DNP_RBSA  (DNP_AHB_OFFSET + 0x110)
#define REG_DNP_RBEA  (DNP_AHB_OFFSET + 0x114)
#define REG_DNP_RBNR  (DNP_AHB_OFFSET + 0x118)
#define REG_DNP_RBNW  (DNP_AHB_OFFSET + 0x11c)


// cmd entry

#define DNP_COMMAND_START_OFFSET 2048 
#define DNP_COMMAND_END_OFFSET   3071


typedef struct dnp_command_s {

/************************1ST WORD**************************************************/  
        uint32_t pad0: 13;
        /*contains the 3D coordinate of the destination DNP*/
        uint32_t dstx: 6;
        uint32_t dsty: 6;
        uint32_t dstz: 6;
	/*contains one of the previous constant: e.g destdev may be equal to 1, then Master M1 is  selected to perform the  command*/
        uint32_t dstdev: 1;
#define DNP_DEV_M0_IF 0
#define DNP_DEV_M1_IF 1

/************************2ND WORD**************************************************/  
	uint32_t len : 32; // len (in bytes)
        
/************************3RD WORD**************************************************/  
        uint32_t pad2: 13;
        uint32_t srcx: 6;
        uint32_t srcy: 6;
        uint32_t srcz: 6;
	/*contains one of the previous constant: e.g srcdev may be equal to 0, then M0 is  selected*/
        uint32_t srcdev: 1;
/************************4TH WORD**************************************************/  
        uint32_t pad4      : 15; 
        uint32_t broadcast : 1; 
        uint32_t comp      : 8; 
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
#define COMP_LAST_PKT     (1<<3) //
#define COMP_RDMA_EVENT   (1<<5) // set by driver to recognize to distinguish a rdma and a buffer event)
        uint32_t cmd       : 8; 
	// desc:
	//   dummy command, do nothing or rise an exception
#define DNP_CMD_NONE             0 
	// desc:
	//   an intra-tile data copy from (srcdev:srcaddr) to
	//   (destdev:destaddr)
	//
	// constraints:
	//   srcdnp==destdnp==localdnp
#define DNP_CMD_MEMCOPY          1
	// desc:
	//   a possibly-remote systolic send-recv.
	//   - local data (srcdev:srcaddr:len) are sent to remote dnp (destdnp)
	//   - remote incoming data from (srcdnp) are moved to local device
	//   (destdev:destaddr)
	//
	// constraints:
	//   len <= max pkt size
#define DNP_CMD_SYSTOLICSNDRCV   2
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
#define DNP_CMD_RDMA_PUT         3
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
#define DNP_CMD_RDMA_GET         4
	// desc:
	//   register the memory area (srcdev:srcaddr:len) as a possible target of
	//   a RDMA_GET or RDMA_PUT
	//
	// constraints:
	//   srcdnp==localdnp
#define DNP_CMD_RDMA_PUBLISH_BUF 5
	// desc:
	//   un-register the previously registered memory area
	//   (srcdev:srcaddr:len)
	//
	// constraints:
	//   srcdnp==localdnp
#define DNP_CMD_RDMA_UNPUBLISH_BUF 6
	
#define DNP_NUM_CMDS (DNP_CMD_RDMA_UNPUBLISH_BUF+1)

/************************5TH WORD**************************************************/  
	uint32_t src_address; //src address


/************************6TH WORD**************************************************/  
	uint32_t dst_address; //dst_address

/************************7TH WORD**************************************************/  
	// if comp&(COMP_CQ|COMP_CQ_FRAG) != 0: this is the magic word written into the Completion Queue
	uint32_t magicw;


} dnp_command_t;





enum {
        NOT_INIT_NET_FOOTER_VAL=0xfef0fef0,
        COMPLETION_WORD_PAD_VAL=0xcec0cec0,
        COMPLETION_DUMMY_FLAGS_VAL=0xcec1cec1,
	COMPLETION_DUMMY_MAGIC_VAL=0xc1c1c1c1,
        RDMA_DUMMY_ADDR=0xf1faf1fa,
        RDMA_DUMMY_MAGIC=0xfedececa,
        // used as magic word in RDMA vaddr_dst
        DNP_STREAMING_BUFFER_MAGIC=0xffffffff,
};
#if 0
enum Registers {

	DNP_REG_NOC_COMMAND = 0,
	DNP_REG_X_PLUS_COMMAND,
	DNP_REG_X_MINUS_COMMAND, 
	DNP_REG_Y_PLUS_COMMAND,
	DNP_REG_Y_MINUS_COMMAND,   		
	DNP_REG_Z_PLUS_COMMAND, 
	DNP_REG_Z_MINUS_COMMAND, 
	DNP_REG_C_COMMAND,	
	DNP_REG_M0_COMMAND,	
	DNP_REG_M1_COMMAND,
	DNP_REG_SWITCH_COMMAND,
	DNP_REG_ARB_ROUT_COMMAND,   
	
	DNP_REG_COMMAND=12, 
	
	DNP_REG_NOC_STATUS,
	DNP_REG_X_PLUS_STATUS,
	DNP_REG_X_MINUS_STATUS,
	DNP_REG_Y_PLUS_STATUS,
	DNP_REG_Y_MINUS_STATUS,
	DNP_REG_Z_PLUS_STATUS,	
	DNP_REG_Z_MINUS_STATUS,
	DNP_REG_C_STATUS,
	DNP_REG_M0_STATUS,	
	DNP_REG_M1_STATUS,	
	DNP_REG_SWITCH_STATUS, //23
	
	DNP_REG_STATUS, 	
     	
	DNP_REG_EXCEPTION,
	DNP_REG_MASK=26,
	
	DNP_REG_3DCOORD_ME, 
	// 1st 6bit (5,0) : Xcoordinate; 2nd 6bit (11,6):Y coordinate; 3th 6bit (17,12):Z coordinate; last 14 bit not in use 
    
	DNP_REG_MYCHIP_3DCOORD,
	// 1st 6bit (5,0) : Xcoordinate; 2nd 6bit (11,6):Y coordinate; 3th 6bit (17,12):Z coordinate; last 14 bit not in use 
	
	DNP_REG_MYCHIP_SIZE,
	// 1st Byte : Size X(in # Tile); 2nd Byte Size Y(in # Tile); 3th Byte Size Z (in # Tile); 4th Byte not in use     
	
	DNP_REG_LATTICE_SIZE=30, 
	// 1st Byte : Size X(in # Tile); 2nd Byte Size Y(in # Tile); 3th Byte Size Z (in # Tile); 4th Byte not in use     
	
	DNP_MAX_WAIT_4DATA,
	DNP_MAX_WAIT_4HEADER, 
	//to avoid deadlock in case a data or header get lost... the intraT_Master will wait max dnp_max_wait_4data/header cycles;
	
	DNP_ARBITRATION_SCHEME=33,  
	
	DNP_ARBITER_PRIORITY_NOC, 
	DNP_ARBITER_PRIORITY_X_PLUS,
	DNP_ARBITER_PRIORITY_X_MINUS,
	DNP_ARBITER_PRIORITY_Y_PLUS,
	DNP_ARBITER_PRIORITY_Y_MINUS,
	DNP_ARBITER_PRIORITY_Z_PLUS,
	DNP_ARBITER_PRIORITY_Z_MINUS,
	DNP_ARBITER_PRIORITY_C,
	DNP_ARBITER_PRIORITY_M0,  
	DNP_ARBITER_PRIORITY_M1=43,  
        
	DNP_ROUTING_TABLE_X,
	DNP_ROUTING_TABLE_Y,
	DNP_ROUTING_TABLE_Z=46,
	
	DNP_ENGINE_PRIORITY_MASTERS, 
	DNP_ENGINE_PRIORITY_SLAVE,
	
	DNP_REG_BLOCK_TRANSFER_M0_FROM_RDMA_CT,
	DNP_REG_BLOCK_TRANSFER_M1_FROM_RDMA_CT,
	DNP_REG_MO_TX_FSM_IDLE,
	DNP_REG_M0_RX_FSM_IDLE,  
	DNP_REG_M1_TX_FSM_IDLE,
	DNP_REG_M1_RX_FSM_IDLE=54, 
	
	RDMA_REG_ENABLE,
	RDMA_REG_LUT_ENABLE,
	RDMA_REG_RB_ENABLE=57,
	
	RDMA_REG_FLAG_LUT_MISS,
	RDMA_REG_FLAG_RB_FULL,
	RDMA_REG_RB_RESTART,
	RDMA_REG_ABORT_CURR_MISSED_REQ,
	RDMA_REQ_ACK_CURR_MISSED_REQ=62,
	
	RDMA_REG_CTDA,
	RDMA_REG_CTDL,
	RDMA_REG_CTRE,
	RDMA_REG_CTMW,
	RDMA_REG_CTFL=67,
	
	RDMA_REG_RBSA,
	RDMA_REG_RBEA,
	RDMA_REG_RBNR,
	RDMA_REG_RBNW=71,	
	
	// ...
	DNP_NUM_REGS,

};
#endif


/* LUT  Don't change these DNP-dependant settings!!*/
#define DNP_RDMA_LUT_NENTRIES  16
#define DNP_RDMA_MAILBOX_NENTRIES  100
#define DNP_LUT_START_OFFSET 1024
#define DNP_LUT_END_OFFSET   2047
#define DNP_LUT_ENTRY_SIZE   16


typedef struct dnp_lut_entry {
        int8_t  *start_addr;
        int8_t  *end_addr;
        uint32_t magicw;
        uint32_t flags;
#define MSK_LUT_FLAG_VALID              1
#define MSK_LUT_FLAG_STREAMING_BUFFER (1<<1) 
#define MSK_LUT_FLAG_GEN_PUT          (1<<2) 
#define MSK_LUT_FLAG_GEN_GETREQ       (1<<3)
#define MSK_LUT_FLAG_GEN_GETRESP      (1<<4)
} dnp_lut_entry_t;

/* Completion event  */

typedef  enum DNP_PKT_HDR_RDMA_CMD {
        DNP_PKT_HDR_RDMA_NONE        = 0,
        DNP_PKT_HDR_RDMA_PUT         = 1,
        DNP_PKT_HDR_RDMA_GET_REQ     = 2,
        DNP_PKT_HDR_RDMA_GET_RESP    = 3,
        DNP_PKT_HDR_RDMA_NCMDS
} DNP_PKT_HDR_RDMA_CMD_T;


#define SIZEOF_CQ_STRUCTURE 8 
typedef struct dnp_completion_event
{
/************************1ST WORD**************************************************/  
    uint32_t cflags :   7;
    uint32_t dstdev :  1;
    uint32_t dstx :    6;
    uint32_t dsty :    6;
    uint32_t dstz :    6;
    uint32_t vchan :    4;
    uint32_t broadcast: 1;
  #define DNP_PKT_HDR_UNICAST       0
  #define DNP_PKT_HDR_BRCAST        1
    uint32_t routing:   1;

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

 
/************************2ND WORD**************************************************/  
    uint32_t pflags : 3;
  /** This field contains protocol dependent flags.
   * It marks the type of protocol packet which is encapsulated inside
   * this packet.
   * LOOPBACK is intended for a simplified test of DNP_CMD_MEMCOPY.
   * MPI proto is not necessary if MPI is implemented on top of RDMA.
   */
  #define DNP_PKT_HDR_PROTO_LOOPBACK 0
  #define DNP_PKT_HDR_PROTO_RDMA     1
  #define DNP_PKT_HDR_PROTO_MPI      2
    uint32_t srcdev : 1;
    uint32_t srcx :   6;
    uint32_t srcy :   6;
    uint32_t srcz :   6;
    uint32_t len :  10;
/** This field contains the payload length, not including header and footer
   * The max pkt size is 1020 bytes
   */  
#define DNP_MAX_PACKET_SIZE 1020
#define DNP_MAX_PAYLOAD_SIZE (DNP_MAX_PACKET_SIZE - sizeof(rdma_pkt_t))

/************************3RD WORD**************************************************/  
    uint32_t rdma_srcx:   6;
    uint32_t rdma_srcy:   6;
    uint32_t rdma_srcz:   6;
    uint32_t rdma_len:   10;
  /** This field contains the length of RDMA operation, not including header and footer.
   * The max length is 1024 bytes but (may be) is limited by max pkt size.
   */
    uint32_t tgtdev:  1;
    uint32_t cmd:     3;
/** This field contains the RDMA command. see enum DNP_PKT_HDR_RDMA_CMD
   */

/************************4th WORD**************************************************/  
/** This field contains the virtual address of the RDMA operation.
   *
   * vaddr is a byte virtual address, to be used by the target master
   * interface to generate a DMA. Unaligned operations are taken care
   * by user defined logic.
   *
   */

    uint32_t src_vaddress : 32; 

/************************5th WORD**************************************************/  
    uint32_t dst_vaddress : 32; // vaddr dest

/************************6th WORD**************************************************/  
    uint32_t crc :   23;
    uint32_t nhops :  8;
    uint32_t err :    1;

  /** This field flags the pkt as corrupted due to crc error.
   * RX state machines set err to 1 in case of crc error, during RX
   * phase.
   */


/************************7th WORD**************************************************/  
    uint32_t lflags : 32; // LUT flags 
/************************8th WORD**************************************************/  
    uint32_t magicw : 32; // LUT completion word


} dnp_completion_event_t;

#define DNP_COMPLETION_NWORDS 8

#endif // __DNP_TYPES_H__

