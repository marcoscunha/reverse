#include <Processor/Processor.h>

void cpu_mp_proceed (void)
{
  cpu_mp_synchro = 0;
  __asm__ volatile ("sync");
}

