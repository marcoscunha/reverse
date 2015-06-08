#include <stdint.h>
#include <Processor/Processor.h>

interrupt_handler_t isr_timer, isr_ipi;

void cpu_trap_attach_isr (int32_t cpuid, interrupt_id_t id,
    uint32_t mode, interrupt_handler_t isr)
{
    switch (id)
    {
    case 0: //IPI
        isr_ipi = isr;
        break;
    case 1: //TIMER
        isr_timer = isr;
        break;
    }
}

