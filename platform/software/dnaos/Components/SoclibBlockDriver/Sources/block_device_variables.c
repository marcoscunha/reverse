#include <Private/SoclibBlockDeviceDriver.h>
#include <Core/Core.h>

driver_t soclib_block_device_module = {
  "soclib_block_device",
  block_device_init_hardware,
  block_device_init_driver,
  block_device_uninit_driver,
  block_device_publish_devices,
  block_device_find_device
} ;

device_cmd_t block_device_commands = {
  block_device_open,
  block_device_close,
  block_device_free,
  block_device_read,
  block_device_write,
  block_device_control
} ;

const char * block_devices[] =
{
  "disk/simulator/0",
  NULL
} ;
