#include <Processor/Processor.h>

void cpu_trap_enable (interrupt_id_t n)
{

}

void cpu_trap_disable (interrupt_id_t n)
{

}

void cpu_trap_attach_esr (exception_id_t id, exception_handler_t isr)
{
  
}

void cpu_trap_attach_isr (interrupt_id_t id,
    uint32_t mode, interrupt_handler_t isr)
{
  
}

interrupt_status_t cpu_trap_mask_and_backup (void)
{
  
  return 0;
}

void cpu_trap_restore (interrupt_status_t backup)
{
  
}
