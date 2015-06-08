/*
 * Copyright (C) 2007 TIMA Laboratory                                    
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

#include <Private/MMC.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

status_t d940_mmc_control (void * handler, int32_t function,
    va_list arguments, int32_t * p_ret)
{
  interrupt_status_t it_status;
  mmc_card_t card = handler;

  watch (status_t)
  {
    ensure (card != NULL, DNA_ERROR);

    it_status = cpu_trap_mask_and_backup ();
    lock_acquire (& card -> lock);

    switch (function)
    {
      case DNA_GET_DEVICE_SIZE :
        {
          int64_t * data = va_arg (arguments, int64_t *);
          ensure (data != NULL, DNA_ERROR);

          *data = card -> info . block_size * card -> info . block_count;
          *p_ret = 0;

          break;
        }

      case DNA_GET_INFO :
        {
          device_info_t * info = va_arg (arguments, device_info_t *);
          ensure (info != NULL, DNA_ERROR);

          *info = card -> info;
          *p_ret = 0;

          break;
        }

      default:
        {
          *p_ret = -1;
          return DNA_ERROR;
        }
    }

    lock_release (& card -> lock);
    cpu_trap_restore (it_status);

    return DNA_OK;
  }
}

