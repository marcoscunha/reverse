#include <stdint.h>
#include <Processor/Processor.h>

int32_t cpu_mp_id (void)
{
  register int32_t id;
  __asm__ volatile ("mfc0 %0, $0" : "=r" (id));
  return id;
}

