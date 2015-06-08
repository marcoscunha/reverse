#ifndef CPU_MP_H
#define CPU_MP_H

#include <platform.h>

extern volatile unsigned long int cpu_mp_synchro;

static inline unsigned long int CPU_MP_ID(void)
{
	register unsigned int id;
  __asm__ volatile ("get %0,rfsl0":"=r"(id));
	return id;
}

#define CPU_MP_COUNT PLATFORM_MP_CPU_COUNT(MIPSR3000)
#define CPU_MP_PROCEED() cpu_mp_synchro = 0
#define CPU_MP_WAIT() while ((volatile unsigned int)cpu_mp_synchro)

#endif

