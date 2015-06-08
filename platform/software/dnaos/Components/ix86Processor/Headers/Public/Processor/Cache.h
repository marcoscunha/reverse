#ifndef PROCESSOR_CACHE_H
#define PROCESSOR_CACHE_H

#include <stdint.h>

typedef enum cpu_cache
{
  CPU_CACHE_INSTRUCTION,
  CPU_CACHE_DATA
}
cpu_cache_t;

#define CPU_CACHE_ALL 0xFFFFFFFF

extern void cpu_cache_sync(void);

/* Caches aren't brain-dead on the intel. */
static inline void flush_cache_all(void) { }

#define cpu_cache_invalidate(cache_type, address, words)    \
do {                                                        \
    flush_cache_all();                                      \
} while (0)

#endif

