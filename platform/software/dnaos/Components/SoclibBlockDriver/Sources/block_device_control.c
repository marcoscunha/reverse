/*
 * Copyright (C) 2011 TIMA Laboratory. Author: H. Chen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Private/SoclibBlockDeviceDriver.h>
#include <DnaTools/DnaTools.h>

status_t block_device_control (void * handler, int32_t function,
    va_list arguments, int32_t * p_ret)
{
  interrupt_status_t it_status;
  block_device_control_t * block_device = (block_device_control_t *)handler;

  watch (status_t)
  {
    ensure (block_device != NULL, DNA_ERROR);

    it_status = cpu_trap_mask_and_backup ();

    switch (function)
    {
      case DNA_GET_DEVICE_SIZE:
        {
          int64_t * p_size = va_arg(arguments, int64_t *);
//          p_size = (int64_t *)arguments;
          ensure (p_size != NULL, DNA_ERROR);

          *p_size = block_device->block_count * block_device->block_size;

          *p_ret = 0;
          break;
        }

      case DNA_GET_INFO:
        {
          device_info_t * info = va_arg (arguments, device_info_t *);
          ensure (info != NULL, DNA_ERROR);

          dna_memset (info, 0, sizeof (device_info_t));
          info->type = DNA_DISK_DEVICE;

          *p_ret = 0;
          break;
        }

      default:
        {
          dna_log(INFO_LEVEL, "Unsupported control code 0x%x.", function);
          *p_ret = -1;
          break;
        }
    }

    cpu_trap_restore (it_status);
    return DNA_OK;
  }
}
