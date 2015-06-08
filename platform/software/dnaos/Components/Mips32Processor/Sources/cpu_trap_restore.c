#include <Processor/Processor.h>

void cpu_trap_restore (interrupt_status_t backup)
{
  interrupt_status_t status = 0;
  __asm__ volatile ("mfc0 %0, $12" : "=r"(status) : );
  status |= (backup & 0x1);
  __asm__ volatile ("mtc0 %0, $12" :  : "r"(status));
}

