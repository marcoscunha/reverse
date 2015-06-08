#include <Processor/Processor.h>

void cpu_context_init (cpu_context_t * ctx, void * sp,
    int32_t ssize, void * entry, void * arg)
{
#if 0
  (*ctx)[3] = (uint32_t)arg;
  (*ctx)[26] = ((uint32_t)sp + (uint32_t)ssize - 4) & 0xfffffff8;
  (*ctx)[28] = (uint32_t)entry;
  (*ctx)[32] = 0xFF01;
#else
  ctx -> gpr[3] = (uint32_t)arg;
  ctx -> gpr[26] = ((uint32_t)sp + (uint32_t)ssize - 4) & 0xfffffff8;
  ctx -> gpr[28] = (uint32_t)entry;
  ctx -> gp_control[3] = 0x2000FF00;
#endif
}

