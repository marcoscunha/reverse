#include <Processor/Processor.h>

interrupt_status_t cpu_trap_mask_and_backup (void)
{
  interrupt_status_t cpsr = 0, cpsr_bak = 0, mask = 0xFFFFFFFE;

  __asm__ volatile ("mfc0 %0, $12" : "=r"(cpsr_bak) : );
  cpsr = cpsr_bak & mask;
  __asm__ volatile ("mtc0 %0, $12" : : "r"(cpsr));

  return cpsr_bak;
}

