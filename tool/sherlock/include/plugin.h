#ifndef _PLUGIN_H_
#define _PLUGIN_H
#include <components.h>

extern int (*pg_init) (int argc, char* argv[]);
extern int (*pg_comp) ( void *comp, void *vals, comp_type_t type);
extern void *(*pg_mem_init) (void);
extern int (*pg_exec_cpu) (void *cpu, void *regs);
extern int (*pg_exec_cache) (void* cache, uint8_t type, uint32_t addr, void* e);
extern int (*pg_exec_mem) (void *mem, void *addr_i, void* inst, uint8_t type, uint32_t addr, void * e);
extern int (*pg_exec_dev) (void *dev);
extern int (*pg_exit) (void);

#define SL_PLUGIN_INIT(plugin_init) \
        int (*pg_init) ( int argc, char *argv[]) = plugin_init;

#define SL_PLUGIN_COMP(plugin_comp) \
        int (*pg_comp) ( void *comp, void *vals, comp_type_t type) = plugin_comp;

#define SL_PLUGIN_MEM_INIT(plugin_mem_init) \
        void *(*pg_mem_init) (void) = plugin_mem_init; 

#define SL_PLUGIN_EXEC_CPU(plugin_exec_cpu) \
        int (*pg_exec_cpu) ( void *cpu, void *regs ) = plugin_exec_cpu;

#define SL_PLUGIN_EXEC_CACHE(plugin_exec_cache) \
        int (*pg_exec_cache) (void* cache, uint8_t type, uint32_t addr, void *e) = plugin_exec_cache;

#define SL_PLUGIN_EXEC_MEM(plugin_exec_mem) \
        int (*pg_exec_mem) ( void *mem, void *addr_i, void *inst, uint8_t type, uint32_t addr, void* e) = plugin_exec_mem;

#define SL_PLUGIN_EXEC_DEV(plugin_exec_dev) \
        int (*pg_exec_dev) (void *dev) = plugin_exec_dev;

#define SL_PLUGIN_EXIT(plugin_exit) \
        int (*pg_exit) ( void ) = plugin_exit;


#endif

