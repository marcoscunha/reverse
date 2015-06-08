#include <stdint.h>
#include <stdbool.h>
#include <Processor/Processor.h>


int32_t atomic_add (int32_t * p_val, int32_t offset)
{
  int32_t old_val = 0, new_val = 0, result = 0;

  do
  {
    DCACHE_INVAL(p_val, sizeof(uint32_t));
    old_val = *p_val;
    new_val = old_val + offset;
    result = cpu_compare_and_swap(p_val, old_val, new_val);
  }
  while (result != 0);

  return old_val;
}

