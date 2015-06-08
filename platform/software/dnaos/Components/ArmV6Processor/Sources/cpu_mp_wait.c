#include <stdint.h>
#include <Processor/Processor.h>
#include <Processor/Cache.h>

void cpu_mp_wait (void)
{
#if defined WRITEBACK || WRITETHROUGH
  DCACHE_INVAL_FAST(&cpu_mp_synchro);
  cpu_mp_synchro = 1;
  DCACHE_FLUSH_FAST(&cpu_mp_synchro);
  /*
   * TODO: add WBFLUSH
   */
  do{
      DCACHE_INVAL_FAST(&cpu_mp_synchro);
  }while ((volatile int32_t)cpu_mp_synchro);
#else 
  cpu_mp_synchro = 1;
  while(cpu_mp_synchro);
#endif
}

