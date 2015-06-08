#include <Processor/Processor.h>

interrupt_status_t cpu_trap_mask_and_backup (void)
{
    int32_t flags = 0;

    __asm__ volatile ("pushf;pop %0;cli": "=a" (flags));

    return flags;
}

