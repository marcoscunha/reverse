#include <stdint.h>
#include <Processor/Processor.h>

int32_t cpu_mp_count (void)
{
    return no_cpus_up;
}

