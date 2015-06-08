#include <stdint.h>
#include <Processor/Processor.h>

volatile int32_t        cpu_mp_synchro = 1;
volatile int32_t        no_cpus_up = 1;
volatile uint32_t       cpus_up_mask = 1;
volatile cpu_ipi_par    cpu_ipi_pars[32];
