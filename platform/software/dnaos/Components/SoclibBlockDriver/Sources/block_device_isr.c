
#include <Private/SoclibBlockDeviceDriver.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

int32_t block_device_isr (void * data)
{
  uint32_t operation_status ;
  
  block_device_control_t block_device = block_device_controls[0] ;
  dna_log(VERBOSE_LEVEL, "Entering ISR") ;

  watch (status_t)
  {
    // ISR ACK
    cpu_read (UINT32, & (block_device . port -> BLOCK_DEVICE_STATUS), operation_status) ;
    ensure (operation_status == BLOCK_DEVICE_READ_SUCCESS || operation_status == BLOCK_DEVICE_WRITE_SUCCESS, DNA_ERROR) ;

    dna_log(VERBOSE_LEVEL, "Operation succeeded.") ;

    semaphore_release (block_device . isr_semaphore_id, 1, DNA_NO_RESCHEDULE) ;
    return DNA_INVOKE_SCHEDULER ;
  }
}
