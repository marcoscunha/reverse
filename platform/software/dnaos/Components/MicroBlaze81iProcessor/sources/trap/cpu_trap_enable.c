#include <cpu.h>

void CPU_TRAP_ENABLE (interrupt_id_t id) {
	interrupt_status_t status;

	__asm__ volatile ("mfs %0, rmsr" : "=r" (status) : );
	status |= 0x2;
	__asm__ volatile ("mts rmsr, %0" : : "r" (status));
}

