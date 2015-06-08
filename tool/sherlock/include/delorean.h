#ifndef _DELOREAN_H_
#define _DELOREAN_H_
#include <core_cpu.h>
#include <core_mem.h>
#include <gdbstub.h>
/**
 *  *  ORACLE 
 *   */
typedef struct { // It consists in a modified values 
    uint8_t     n_cpu;
    uint8_t     n_mem;
    uint8_t     n_dev;
    cpu_t       **cpu;
    mem_t       **mem;
    mem_t       **dev;
    inst_node_t *last_exec;
}oracle_t;

void        oracle_reverse_exec(gdb_param_t* param, comp_type_t *type);
void        oracle_forward_exec(gdb_param_t* param, comp_type_t *type);
cpu_t       *oracle_get_cpu(uint32_t id);
uint16_t    oracle_get_ncpu(void);

bool        oracle_get_trace(void);
inst_node_t *oracle_get_last_exec(void);

void output_partial_results(void);



#endif
   
