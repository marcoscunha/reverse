#ifndef CPU_CONTEXT_H
#define CPU_CONTEXT_H

#include <stdint.h>

typedef struct CPU_CONTEXT {
	uint32_t GPR[31];
	uint32_t MSR;
} CPU_CONTEXT_T;

#define CPU_CONTEXT_SIZE sizeof(CPU_CONTEXT_T) 

extern void CPU_CONTEXT_INIT (CPU_CONTEXT_T *, void *, int32_t, void *, void *);
extern void CPU_CONTEXT_LOAD (CPU_CONTEXT_T *);
extern void CPU_CONTEXT_SAVE (CPU_CONTEXT_T *);
extern void CPU_CONTEXT_SWITCH (CPU_CONTEXT_T *, CPU_CONTEXT_T *);

#endif

