#include <cpu.h>

interrupt_status_t CPU_TRAP_MASK_AND_BACKUP (void) {
	interrupt_status_t status, backup;

	__asm__ volatile ("mfs %0, rmsr" : "=r" (backup) : );
	status = backup & 0xFFFFFFFD;
	__asm__ volatile ("mts rmsr, %0" : : "r" (status));

	return backup;
}

