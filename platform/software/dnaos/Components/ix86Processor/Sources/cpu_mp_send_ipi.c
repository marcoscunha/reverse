#include <Processor/Processor.h>
#include <Platform/Platform.h>
#include <PCPlatformDriver/Driver.h>
#include <Processor/apic_regs.h>

void cpu_mp_send_ipi (int32_t target, int32_t command, void * data)
{
    cpu_ipi_pars[target].command = command;
    cpu_ipi_pars[target].data = data;
    cpu_ipi_pars[target].status = 1;

    local_apic_mem[LAPIC_ICR_HIGH >> 2] = target << 24;
    local_apic_mem[LAPIC_ICR_LOW >> 2] = 0x00004000 + LAPIC_IPI_VECTOR;
}

