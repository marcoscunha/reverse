#include <Processor/Processor.h>

void cpu_power_wake_on_interrupt (void)
{
    __asm__ volatile ("hlt");
}

