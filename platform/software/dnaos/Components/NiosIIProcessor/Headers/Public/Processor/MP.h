#ifndef PROCESSOR_MP_H
#define PROCESSOR_MP_H

#include <Platform/Platform.h>

extern volatile unsigned long int cpu_mp_synchro;

static inline unsigned long int cpu_mp_id(void)
{
  register unsigned int id;
  __asm__ volatile ("rdctl %0, cpuid" : "=r"(id));
  return id;
}

#define cpu_mp_count platform_mp_cpu_count(NIOS2FAST)
#define cpu_mp_proceed() cpu_mp_synchro = 0
#define cpu_mp_wait() while ((volatile unsigned int)cpu_mp_synchro)

#endif

