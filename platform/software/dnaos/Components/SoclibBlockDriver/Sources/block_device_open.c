
#include <Private/SoclibBlockDeviceDriver.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>


status_t block_device_open (char * name, int32_t mode, void ** data)
{
  
  block_device_control_t * block_device_p = block_device_controls ;
  *data = (void *) block_device_p ;
  return DNA_OK ;
}

