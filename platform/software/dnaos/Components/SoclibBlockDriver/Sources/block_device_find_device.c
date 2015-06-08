
#include <Private/SoclibBlockDeviceDriver.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>


device_cmd_t * block_device_find_device (const char * name)
{
  return & block_device_commands ;
}
