#include <cpu.h>

#define CPU_FIRST_ARG_REG  	 4
#define CPU_SECOND_ARG_REG 	 5
#define CPU_STACK_PTR_REG  	 0
#define CPU_RETURN_ADDR_REG	 14

void CPU_CONTEXT_INIT (CPU_CONTEXT_T * ctx, void * sp, int32_t ssize, void * entry, void * arg) {
	ctx -> GPR[CPU_STACK_PTR_REG] = ((uint32_t)sp + (uint32_t)ssize - 4 - 512) & 0xffffff00;
	ctx -> GPR[CPU_RETURN_ADDR_REG] = (uint32_t)(entry - 8);
	ctx -> GPR[CPU_FIRST_ARG_REG] = (uint32_t)arg;

	/* Enable data and instruction cache*/
	ctx-> MSR = 0x000000A0;
}
