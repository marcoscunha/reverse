
#include <Private/SoclibBlockDeviceDriver.h>
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

void block_device_uninit_driver (void)
{
  int32_t idx ;
  block_device_control_t block_device ;
  
  for (idx = 0 ; idx < SOCLIB_BLOCK_DEVICES_NDEV ; idx++)
  {
    block_device = block_device_controls[idx] ;
    if (block_device . should_enable_irq) {
      interrupt_detach (0, block_device . irq, block_device_isr) ;
    }
  }

  kernel_free (block_device_controls) ;

}
