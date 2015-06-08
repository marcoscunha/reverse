#ifndef PROCESSOR_CONTEXT_H
#define PROCESSOR_CONTEXT_H

#include <stdint.h>

#if 0
typedef volatile uint32_t cpu_context_t [33];
#else
typedef volatile struct _cpu_context
{
  uint32_t fpr[32];
  uint32_t fp_control[5];
  uint32_t gpr[29];
  uint32_t gp_control[4];
}
cpu_context_t;
#endif

#define CPU_CONTEXT_SIZE sizeof(cpu_context_t) 

extern void cpu_context_init (cpu_context_t *, void *, int32_t, void *, void *);
extern void cpu_context_save (cpu_context_t *, uint32_t *);
extern void cpu_context_load (cpu_context_t *);

#endif

