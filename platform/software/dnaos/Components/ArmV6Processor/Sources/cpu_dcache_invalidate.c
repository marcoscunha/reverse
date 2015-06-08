#include <stdint.h>
#include <Processor/Processor.h>

inline void cpu_dcache_invalidate(void * address, int32_t bytes)
{
    register int32_t count = 0;
    register uint32_t adjusted_address = (uint32_t)address & 0xFFFFFFE0UL; // Based on cache line size

    if (bytes == CPU_CACHE_ALL)
    {
        __asm__ volatile ("mcr p15, 0, %0, c7, c6, 0" : : "r"(count));

    }
    else
    {
        count = bytes >> CPU_DCACHE_SIZE_LOG2;
        count += (bytes - (count << CPU_DCACHE_SIZE_LOG2) != 0) ? 1 : 0;

        if((((uint32_t)address + (bytes-1)) & 0x1F) < (bytes & 0x1FUL)){
            count++;
        }


        for (register int32_t i = 0; i < count; i += 1)
        {
            __asm__ volatile ("mcr p15, 0, %0, c7, c6, 1"
                    :
                    : "r"(adjusted_address));
            adjusted_address += 1 << CPU_DCACHE_SIZE_LOG2;
        }
    }
}
