#include <Processor/Processor.h>

void cpu_mp_proceed (void)
{
  cpu_mp_synchro = 0;
  DCACHE_FLUSH_FAST(&cpu_mp_synchro);
  /*
   * TODO: add the WBFLUSH
   */
}

