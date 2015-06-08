#include <stdint.h>
#include <cpu.h>

void CPU_TRAP_ATTACH_ISR (interrupt_id_t id, uint32_t mode, interrupt_handler_t isr) {
	handler_table[0] = isr;
}

