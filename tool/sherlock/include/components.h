#ifndef _COMPONENTS_H_
#define _COMPONENTS_H_

#include<core_cpu.h>
#include<core_cache.h>
#include<core_mem.h>

typedef enum comp_type_t{
    COMP_CPU,
    COMP_CACHE,
    COMP_MEM,
    COMP_PERIPH,
} comp_type_t;

typedef union oracle_comp_t{
    cpu_t   *cpu;
    cache_t *cache;
    mem_t   *mem;
}oracle_comp_t;

typedef struct common_comp_t{
    uint8_t        comp_type;       /*Must be the first field in all components */
    hwe_id_dev_t   id;
}common_comp_t;

#endif // _COMPONENTS_H_

