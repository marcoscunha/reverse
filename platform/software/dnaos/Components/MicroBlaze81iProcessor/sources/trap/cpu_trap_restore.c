#include <cpu.h>

void CPU_TRAP_RESTORE (interrupt_status_t backup) {
	interrupt_status_t status;

	__asm__ volatile ("mfs %0, rmsr" : "=r" (status) : );
	status = status | (backup & 0x2);
	__asm__ volatile ("mts rmsr, %0" : : "r" (status));
}

