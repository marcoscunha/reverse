#ifndef _CFG_H_
#define _CFG_H_
#include <hwetrace.h>
//#include <components.h>


/**
 *  DEVICE STRUCTURES 
 */
typedef struct{
    uint8_t type;
    uint8_t id;    // up to 256 processors
}dev_data_t;
/**
 *  MEMORY STRUCTURES 
 */
typedef struct m_node{
    uint8_t     type;
    uint8_t     id;    // up to 256 processors
    uint32_t    addr;
    uint32_t    value;
    struct m_node *last_write;
    struct i_node *instr;
}mem_data_t;

/*typedef struct m_node_t{
    mem_data_t    *data;
    struct i_node *last_write;
} mem_node_t;
*/
/**
 * REGISTER STRUCTURES
 */
typedef struct{
   uint8_t       id;
   uint32_t      value;
   struct i_node *last_write;
}reg_node_t;

/**
 * INSTRUCTIONS STRUCTURES
 */
typedef struct{
    uint8_t       type;
    uint8_t       id;    // up to 256 processors
    uint8_t       n_reg; 
    uint8_t       n_mem;
    reg_node_t    *reg;
    struct i_node **mem;
}data_t;



typedef struct i_node {
    union {
        data_t     *inst;
        mem_data_t *mem; 
        dev_data_t *dev;
    }data;

    struct i_node *next;
    struct i_node *last_exec;
} inst_node_t;

/**
 * COMPONENT DEFINITIONS
 */
#define REG_METADATA inst_node_t *last_write
#define CPU_METADATA inst_node_t *last_exec; \
                     inst_node_t *next_exec;
#define MEM_METADATA mem_data_t *last_write 
#define MEMDEV_METADATA inst_node_t *last_exec; \
                        inst_node_t *next_exec;


#endif
