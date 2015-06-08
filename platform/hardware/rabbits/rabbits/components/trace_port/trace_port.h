#ifndef _TRACE_PORT_
#define _TRACE_PORT_

#include <hwetrace.h>
#include <hwetrace_api.h>
#include <events/hwe_device.h>
#include <events/hwe_common.h>

#define TR_BUF_READ  0
#define TR_BUF_WRITE 1

#define TR_LIST_SIZE 0x80

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * ...
 */
typedef enum
{
    TR_EVNT_TRACE_NULL,     /**< Processor NULL TRACE                                     */
    TR_EVNT_PROC_INST,      /**< Processor Instruction                                    */
    TR_EVNT_PROC_DCACHE_REQ,/**< Processor Request Instruction to Data Cache Memory       */
    TR_EVNT_PROC_ICACHE_REQ,/**< Processor Request Instruction to Instruction Cache Memory*/
    TR_EVNT_ICACHE_REQ,     /**< Instruction Cache Request to Main Shared Memory          */
    TR_EVNT_DCACHE_REQ,     /**< Data Cache Request to Main Shared Memory                 */
    TR_EVNT_ICACHE_ACK,     /**< Instruction Cache Acknowledge to Processor Request       */
    TR_EVNT_DCACHE_ACK,     /**< Data Cache Acknowledge to Processor Request              */
    TR_EVNT_ICACHE_REPL,    /**< Hardware replace instruction cache line                  */
    TR_EVNT_DCACHE_REPL,    /**< Hardware replace data cache line                         */
    TR_EVNT_ICACHE_INV,     /**< Hardware invalidate instruction cache line               */
    TR_EVNT_DCACHE_INV,     /**< Hardware invalidate data cache line                      */
    TR_EVNT_ICACHE_SW_INV,  /**< Software invalidate instruction cache line               */
    TR_EVNT_DCACHE_SW_INV,  /**< Software invalidate data cache line                      */
    TR_EVNT_ICACHE_SW_FLUSH,/**< Software flush instruction cache line                    */
    TR_EVNT_DCACHE_SW_FLUSH,/**< Software flush data cache line                           */
    TR_EVNT_DCACHE_WRITE_HIT,/**< Data Cache Write event when cache Hit occurs            */
    TR_EVNT_DCACHE_WRITE_MISS,/**< Data Cache Write event when cache Miss occurs          */
    TR_EVNT_DCACHE_MODIFY,  /**< Data Cache Modify event                                  */
    TR_EVNT_DCACHE_WRITEBACK,/**< Data Cache WriteBack                                    */
    TR_EVNT_MEMORY_ACK,     /**< Main Shared Memory Acknowledge from Cache Request        */
    TR_EVNT_PROC_IO_REQ,    /**< Processor Request to IO Memory Mapped devices            */
    TR_EVNT_IO_ACK,         /**< IO Memory Mapped Device Acknowledge to Processor Request */
    TR_EVNT_COMMIT          /**< Event to commit the QEMU event into HWE_TRACE            */
}tr_event_grp_t;

/**
 * Address type. This could be change for bigger address architectures.
 */
typedef uint32_t tr_addr_t;

/**
 * Identification type. This can be configured for bigger identification numbers
 */
typedef uint64_t tr_id_t;


typedef uint32_t tr_cycle_t;

typedef enum {
    TR_ARM_NORMAL_INSTRUCTION,
    TR_ARM_JUMP,
    TR_ARM_LOAD,
    TR_ARM_STORE,
    TR_ARM_SIGNED_MUL,
    TR_ARM_REGISTER_SHIFT,
    TR_ARM_MUL8,
    TR_ARM_MUL16,
    TR_ARM_MUL24,
    TR_ARM_MUL32,
    TR_ARM_MUL64,
    TR_ARM_MLS,
    TR_ARM_MLA,
    TR_ARM_MLAA,	/* ? */
    TR_ARM_SWP,
    TR_ARM_MULTI_TRANSFER_PER_REGISTER,
    TR_ARM_MULTI_TRANSFER_LOAD_OP,
    TR_ARM_MULTI_TRANSFER_STORE_OP,
    TR_ARM_COCPU,	/* ? */
    TR_ARM_COCPU_MRC,
    TR_ARM_INSTR_CYCLES_NO_ITEMS
}tr_inst_t;

/**
 * @struct
 */
typedef enum tr_mem_t { // The same as hwe_mem_t in Decopus (trace/include/events/hwe_mem.h)
   TR_MEM_LOAD,    /**< load                                                  */
   TR_MEM_STORE,   /**< store                                                 */
   TR_MEM_MODIFY,  /**< modify                                                */
   TR_MEM_WRITEBACK,/**< writeback                                            */
   TR_MEM_LL,      /**< load linked                                           */
   TR_MEM_SC,      /**< store conditional                                     */
   TR_MEM_PREF,    /**< prefetch (no data)                                    */
   TR_MEM_SYNC,    /**< sync (proc) (no data)                                 */
   TR_MEM_REPL,    /**< replace (to caches) (no data)                         */ 
   TR_MEM_INVAL,   /**< invalidate (to caches) (no data)                      */
   TR_MEM_SW_INVAL,/**< invalidate (to caches) using instruction (no data)    */
   TR_MEM_SW_FLUSH,/**< flush (to caches) using instruction (no data)         */
} tr_mem_t;


typedef struct{ //
    uint32_t   pc;       /**< Program Counter                                 */
    uint32_t   insn;     /**< Instruction at PC                               */
    tr_inst_t  inst_grp; /**< Instruction group                               */
    tr_cycle_t cycle;    /**< Number of cycles to execute this instruction    */
    uint32_t   pwr;      /**< Power consumption for instruction               */
    uint32_t   jmp_addr; /**< Jump instruction address                        */
    uint32_t   dmem_access:16; /**< Number of data memory access                 */
    uint32_t   imem_access:16; /**< Number of instruction memory access          */
    uint32_t   unaligned_access:1; /**< Instruction with unaligned access to mem*/
    uint32_t   dmem_ld_ex:1;  /**< Load Exclusive  Instruction                  */
    uint32_t   dmem_st_ex:1;  /**< Store Exclusive Instruction                  */
    uint32_t   cond:1;        /**< Instruction under conditional execution      */
    uint32_t   exec:1;        /**< Instruction Executed after condition analysis*/
}tr_event_inst_t;

typedef struct {
    tr_id_t    src_id;   /**< ID of instruction or cache event                */
    tr_addr_t  addr;     /**< Address accessed                                */
    uint16_t   width;    /**< Data Width Access => XXX: Really necessary ?    */
    tr_mem_t   access;   /**< Type of Access: Read, Write, Lock, ...          */
}tr_event_req_t ;

typedef struct{
    tr_id_t    src_id;   /**< ID of request event                             */
    tr_addr_t  addr;     /**< Address accessed                                */
    uint32_t   order;    /**< Incremental time for acknowledge memory events  */
}tr_event_ack_t;

typedef struct{
    tr_id_t    src_id;   /**< ID of last event of a qemu's instruction        */
    uint32_t   pc_after;   /**< Incremental time for acknowledge memory events  */
}tr_event_com_t;

/**
 * @struct A struct to ...
 */
typedef union{
	tr_event_inst_t inst; /**< Instruction event type                         */
	tr_event_req_t  req;  /**< Request event type                             */
	tr_event_ack_t  ack;  /**< Acknowledge event type                         */
	tr_event_com_t  com;  /**< Commit event type                              */
}tr_data_t;

/**
 * @struct The struct of events
 */
typedef struct{
    uint64_t       id;       /**< Unique trace identifier                     */
    tr_event_grp_t type;     /**< Kind of event trace                         */
    uint16_t       cpu;      /**< CPU number                                  */
    uint64_t       timestamp;/**< Cycle when event acknowledge occurred       */
    tr_data_t      data;     /**< Event information                           */
}tr_event_t;


typedef struct{
   hwe_port_t* cpu;
   hwe_port_t* dcache;
   hwe_port_t* icache;
}trace_port_t;

#ifdef __cplusplus
}
#endif

#endif

