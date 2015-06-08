
#include <Private/SoclibBlockDeviceDriver.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Cache.h>

status_t access_device_blocks(block_device_control_t * block_device, 
    void * access, int64_t block_offset, int32_t block_count,
    block_device_access_t read_or_write) 
{
  uint32_t operation_status ;
  interrupt_status_t it_status ;

  watch (status_t)
  {
    it_status = cpu_trap_mask_and_backup() ;
    cpu_write (UINT32, & (block_device -> port -> BLOCK_DEVICE_BUFFER),
        access) ;
    dna_log(VERBOSE_LEVEL, "Wrote destination buffer address: 0x%x", 
        access) ;
    
    cpu_write (UINT32, & (block_device -> port -> BLOCK_DEVICE_COUNT),
        block_count) ;
    dna_log(VERBOSE_LEVEL, "Wrote block count: %d", block_count) ;

    cpu_write (UINT32, & (block_device -> port -> BLOCK_DEVICE_LBA),
        block_offset) ;
    dna_log(VERBOSE_LEVEL, "Wrote block offset (LBA): %d", block_offset) ;

    if (block_device -> should_enable_irq)
    {
      cpu_write (UINT32, & (block_device -> port -> BLOCK_DEVICE_IRQ_ENABLE),
          (uint32_t) true) ;
      cpu_write (UINT32, & (block_device -> port -> BLOCK_DEVICE_OP), 
          read_or_write == READ ? BLOCK_DEVICE_READ : BLOCK_DEVICE_WRITE) ;


      cpu_trap_restore (it_status) ;
      semaphore_acquire (block_device -> isr_semaphore_id, 1, 0, -1) ;
    }
    else
    {
      cpu_write (UINT32, & (block_device -> port -> BLOCK_DEVICE_IRQ_ENABLE),
          (uint32_t) false) ;
      cpu_write (UINT32, & (block_device -> port -> BLOCK_DEVICE_OP), 
          read_or_write == READ ? BLOCK_DEVICE_READ : BLOCK_DEVICE_WRITE) ;

      do {
        cpu_read (UINT32, & (block_device -> port -> BLOCK_DEVICE_STATUS), 
            operation_status) ;
        dna_log(VERBOSE_LEVEL, "Op status is %d.", operation_status) ;
      } while (operation_status == BLOCK_DEVICE_BUSY) ;

      cpu_trap_restore (it_status) ;
      ensure (operation_status == BLOCK_DEVICE_READ_SUCCESS ||
          operation_status == BLOCK_DEVICE_WRITE_SUCCESS, DNA_ERROR) ;
    }
    return DNA_OK ;
  }
}
