#ifndef _SHERLOCK_H_
#define _SHERLOCK_H_
#include <cfg.h>
#include <hwetrace.h>
#include <events/hwe_events.h>
#include <events/hwe_device.h>
#include <hwe_handle_def.h>


#include <core_cpu.h>
#include <core_mem.h>
#include <core_cache.h>

#include <components.h>
#include <plugin.h>


struct comp_data_t {
    evfifo_t fifo;    // fifo
    union {
       cpu_t   *cpu;       // cpu 
       mem_t   *mem;
       cache_t *cache;
    } comp;
//   put a plugin-dependent structure
};

//struct comp_data {
    /*
     * one fifo to enforce the component trace order on some event
     */
//    evfifo_t fifo;
//    hwe_device_t type;
    /*
     * type dependent field
     */
//    union {
//        struct {
//            int cpuid;
//            vmc_cpu_t *cpu;
//        } cpu;
//        struct {
//            vmc_cache_t *cache;
//        } cache;
//        struct {
//            vmc_access_t *first;
//            vmc_access_t *last;
//            vmc_addr_t    addr;
//        } buffer;
//        struct {
//            bool cached;
//        } segment;
//    } u;
//};

typedef enum {
    EVENT_NOOP = 0,
    EVENT_INST,
    EVENT_READ,
    EVENT_WRITE,
    EVENT_MERGE,
    EVENT_FILL,
    EVENT_DEVICE_FILL,
    EVENT_INVAL,
    EVENT_ACK_READ,
    EVENT_ACK_WRITE,
    EVENT_ACK_MODIFY,
}event_data_type_t;

struct event_data_t {
    event_data_type_t type;
    exec_reg_t regs;
//#ifdef EVENT_DATA_METADATA 
    inst_node_t *inst_node;
//  EVENT_DATA_METADATA
//#endif
};

//struct event_data {
    /*
     * type of the request
     */
//    enum event_data_type type;
    /*
     * type dependent field
     */
//    union {
        /*
         * instruction
         */
//        vmc_inst_t inst;
        /*
         * simple read/write access
         */
//        struct {
            /* read access (from cpu to segment/cache/buffer) */
//            vmc_addr_t addr;
            /* read access (from cpu to segment/cache/buffer) */
//            vmc_access_t *access;
//        } rw;
        /*
         * sevral merged writes access
         */
//        struct {
            /* read access (from cpu to segment/cache/buffer) */
//            vmc_addr_t addr;
            /* first merged write (from buffer to segment) */
//            vmc_access_t *first;
            /* last merged write (from buffer to segment) */
//            vmc_access_t *last;
//        } merge;
        /*
         * cache fill request
         */
//        struct {
            /* address (from cache to segment) */
//            vmc_addr_t addr;
            /* write access being fetched (from segment to cache) */
//            vmc_access_t *access;
//        } fill;

        /*
         * inval
         */
//        struct {
            /* address (from cpu to cache) */
//            vmc_addr_t addr;
//        } inval;
//    } u;
//};

//
#define COMP_DATA_T struct comp_data_t
#define EVENT_DATA_T struct event_data_t

//used callback stages
#define USE_STAGE_GO
#define USE_STAGE_RET

#include <hwe_handle_header.h>

void cb_nop(event_t *e);


#endif // _SHERLOCK_H_

