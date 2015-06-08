#ifndef _CFG_H_
#define _CFG_H_
#include <hwetrace.h>
//#include <components.h>

#define MAX_PENDING 10*1024*1024

#define SMALL_MEMORY

/**
 *  DEVICE STRUCTURES 
 */
typedef struct{
}dev_data_t;

typedef struct writer_t{
    uint8_t  id;
    uint8_t  cpu_id;
    uint32_t pc;
    uint64_t pre_order;
    uint64_t pos_order;
#ifndef SMALL_MEMORY
    uint64_t comp_order;
    uint64_t cpu_index;
    uint64_t timestamp;
#endif
}writer_t;

typedef writer_t modifier_t;
typedef writer_t reader_t;

/*typedef struct reader{
    uint8_t  id;
    uint8_t  cpu_id;
    uint16_t spare;
    uint32_t pc;
    uint64_t pre_order;
    uint64_t pos_order;
    uint64_t comp_order;
    uint64_t cpu_index;
//  uint64_t timestamp;
}reader_t;*/


/**
 *  CACHE STRUCTURES 
 */

typedef struct cache_data_t{
    uint64_t  g_order;
    uint32_t  n_pend_read;
    uint32_t  n_pend_mod;
    struct pend_read_t{
        uint32_t addr;
        reader_t reader;
    } pend_read[MAX_PENDING];
    struct pend_mod_t{
        uint32_t addr;
        modifier_t modifier;
    } pend_mod[MAX_PENDING];
    struct cache_stat_t{
        uint32_t max_n_pend_read;
        uint32_t max_n_pend_mod;
        uint32_t false_positive;
    }stats;

}cache_data_t;

/**
 *  MEMORY STRUCTURES 
 */

typedef struct m_node{
    uint8_t    state;
    uint8_t    n_readers;
    uint32_t   bitmap_readers;  // A bitmap with readers
    writer_t   writer;
    reader_t   reader;
    modifier_t modifier;
    bool       bad_addr;
    bool       warn_addr;
//  reader_t  readers[256];   
//    uint32_t  expected_readers;
//    uint32_t  expected_writers;
}mem_data_t;


/**
 * REGISTER STRUCTURES
 */
typedef struct{
}reg_node_t;

/**
 * INSTRUCTIONS STRUCTURES
 */
typedef struct{
}data_t;

typedef struct i_node {
} inst_node_t;

/**
 * COMPONENT DEFINITIONS
 */
//#define REG_METADATA 
//#define CPU_METADATA 
#define CACHE_METADATA cache_data_t cache_data;
#define MEM_METADATA mem_data_t mem_data; 
//#define MEMDEV_METADATA 


#endif
