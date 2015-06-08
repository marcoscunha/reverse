
#include <Private/SoclibBlockDeviceDriver.h>
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

block_device_control_t * block_device_controls ;

status_t block_device_init_driver (void)
{
  int32_t i ;
  char alpha_num[8], semaphore_name_buffer[64], * semaphore_prefix = "block_device_" ;
  char isr_semaphore_name[64] ;
  
  watch (status_t)
  {
    block_device_controls = kernel_malloc (sizeof (block_device_control_t) * 
        SOCLIB_BLOCK_DEVICES_NDEV, true) ;
    ensure (block_device_controls != NULL, DNA_OUT_OF_MEM) ;
    

    for (i = 0 ; i < SOCLIB_BLOCK_DEVICES_NDEV ; i++)
    {
      dna_itoa (i, alpha_num) ;
      dna_strcpy (semaphore_name_buffer, semaphore_prefix) ;
      dna_strcat (semaphore_name_buffer, alpha_num) ;
      dna_strcat (semaphore_name_buffer, "_sem") ;

      semaphore_create (semaphore_name_buffer, 1, 
          &block_device_controls[i] . semaphore_id) ;

      block_device_controls[i] . should_enable_irq = 
        (bool) SOCLIB_BLOCK_DEVICES[i] . should_enable_irq ;
      block_device_controls[i] . port = 
        (block_device_register_map_t) SOCLIB_BLOCK_DEVICES[i] . base_address ;

      cpu_read (UINT32, 
          & (block_device_controls[i] . port -> BLOCK_DEVICE_SIZE), 
          block_device_controls[i] . block_count) ;
      cpu_read (UINT32, 
          & (block_device_controls[i] . port -> BLOCK_DEVICE_BLOCK_SIZE), 
          block_device_controls[i] . block_size) ;

      if (block_device_controls[i] . should_enable_irq) {
        block_device_controls[i] . irq = SOCLIB_BLOCK_DEVICES[i] . irq ;
        interrupt_attach (0, SOCLIB_BLOCK_DEVICES[i] . irq, 0x0, 
            block_device_isr, false) ;
        dna_strcpy (isr_semaphore_name, semaphore_name_buffer) ;
        dna_strcat (isr_semaphore_name, "_isr") ;
        semaphore_create (isr_semaphore_name, 0, 
            &block_device_controls[i] . isr_semaphore_id) ;
      }

    }

    return DNA_OK ;
  }
}
