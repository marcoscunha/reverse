#ifndef PROCESSOR_CACHE_H
#define PROCESSOR_CACHE_H

#include <stdint.h>

typedef enum cpu_cache
{
  CPU_CACHE_INSTRUCTION,
  CPU_CACHE_DATA
}
cpu_cache_t;

//#define CPU_ICACHE_SIZE_LOG2 5
//#define CPU_DCACHE_SIZE_LOG2 5

#define CPU_ICACHE_SIZE_LOG2 2
#define CPU_DCACHE_SIZE_LOG2 2
//#define CPU_DCACHE_MASK      0xFFFFFFE0UL
#define CPU_DCACHE_MASK      0xFFFFFFFCUL



#define CPU_CACHE_ALL 0xFFFFFFFF


#define cpu_cache_sync()                      \
{                                             \
/*      volatile register int32_t dummy = 0; */ \
                                              \
  __asm__ volatile (                          \
      "mcr p15,0,%0,c7,c10,4"                 \
      :                                       \
      : "r"(0)                                \
      );                                      \
}

extern void cpu_cache_invalidate (cpu_cache_t cache_type,
    void * address, int32_t words);

extern void cpu_dcache_invalidate       (void* address, int32_t bytes);
//extern void cpu_dcache_flush            (void *address, int32_t bytes);
//extern void cpu_dcache_flush_invalidate (void *address, int32_t bytes);

static inline void cpu_dcache_flush(register void * address, register int32_t bytes)
{

    register int32_t count = 0;
    register uint32_t adjusted_address = (uint32_t)address & CPU_DCACHE_MASK; // Based on cache line size

    if (bytes == CPU_CACHE_ALL)
    {
        __asm__ volatile ("mcr p15, 0, %0, c7, c10, 0" : : "r"(0));
    }
    else
    {
        count = bytes >> CPU_DCACHE_SIZE_LOG2;
        count += ((bytes & ~CPU_DCACHE_MASK) != 0) ? 1 : 0;
        count += ((((uint32_t)address + (bytes-1)) & ~CPU_DCACHE_MASK) < \
                   ((uint32_t)address & ~CPU_DCACHE_MASK)) ? 1: 0;

        for (register int32_t i = 0; i < count; i += 1)
        {
            __asm__ volatile ("mcr p15, 0, %0, c7, c10, 1"
                    :
                    : "r"(adjusted_address));
            adjusted_address += 1 << CPU_DCACHE_SIZE_LOG2;
        }
    }
}


static inline void cpu_dcache_flush_invalidate(register void * address, register int32_t bytes)
{
    register int32_t count = 0;
    register uint32_t adjusted_address = (uint32_t)address &CPU_DCACHE_MASK; // Based on cache line size

    if (bytes == CPU_CACHE_ALL)
    {
        __asm__ volatile ("mcr p15, 0, %0, c7, c14, 0" : : "r"(count));

    }
    else
    {
        register int32_t i;
        count = bytes >> CPU_DCACHE_SIZE_LOG2;
        count += ((bytes  & ~CPU_DCACHE_MASK) != 0) ? 1 : 0;
        count += ((((uint32_t)address + (bytes-1)) & ~CPU_DCACHE_MASK) < \
                   ((uint32_t)address & ~CPU_DCACHE_MASK)) ? 1 : 0;

        for ( i = 0; i < count; i += 1)
        {
            __asm__ volatile ("mcr p15, 0, %0, c7, c14, 1"
                    :
                    : "r"(adjusted_address));
            adjusted_address += 1 << CPU_DCACHE_SIZE_LOG2;
        }
    }
}


#ifdef WRITEBACK
#define DCACHE_FLUSH(_addr,_bytes) cpu_dcache_flush((void*)_addr, _bytes)
#define DCACHE_INVAL(_addr,_bytes) cpu_dcache_flush_invalidate ((void *)_addr, _bytes)

#define DCACHE_FLUSH_FAST(_addr) __asm__ volatile ("mcr p15, 0, %0, c7, c10, 1":: "r"(_addr));
#define DCACHE_INVAL_FAST(_addr) __asm__ volatile ("mcr p15, 0, %0, c7, c14, 1":: "r"(_addr));

#define DCACHE_INVAL_LOST(_addr) __asm__ volatile ("mcr p15, 0, %0, c7, c6, 1":: "r"(_addr));


#elif defined WRITETROUGH
#define DCACHE_FLUSH(_addr,_bytes) 
#define DCACHE_INVAL(_addr,_bytes) cpu_dcache_invalidate ((void *)_addr, _bytes)

#define DCACHE_FLUSH_FAST(_addr)
#define DCACHE_INVAL_FAST(_addr) __asm__ volatile ("mcr p15, 0, %0, c7, c6, 1":: "r"(_addr));

#define DCACHE_INVAL_LOST(_addr)  DCACHE_INVAL_FAST(_addr);

#else
#define DCACHE_FLUSH(_addr,_bytes) 
#define DCACHE_INVAL(_addr,_bytes)
#define DCACHE_FLUSH_FAST(_addr)
#define DCACHE_INVAL_FAST(_addr)
#define DCACHE_INVAL_LOST(_addr)
#endif  

#endif

