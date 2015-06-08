#include <stdint.h>
#include <Processor/Processor.h>

int32_t cpu_mp_id (void)
{
  register int32_t id = 0;

  __asm__ volatile (
      "mrc p15,0,%0,c0,c0,5"
      : "=r"(id)
      :
      );

  return id;
}
