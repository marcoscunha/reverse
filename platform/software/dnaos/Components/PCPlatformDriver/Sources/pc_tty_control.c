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

#include <Private/Driver.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

status_t pc_tty_control (void * handler __attribute__((unused)),
						 int32_t function, va_list arguments, int32_t * p_ret)
{
  switch (function)
  {
    case DNA_GET_INFO :
      {
        device_info_t * info = va_arg (arguments, device_info_t *);

        dna_memset (info, 0, sizeof (device_info_t));
        info -> type = DNA_CHARACTER_DEVICE;

        *p_ret = 0;
        break;
      }

    default :
      {
        dna_log(INFO_LEVEL, "Unsupported control code 0x%x.", function);
        *p_ret = -1;
        break;
      }
  }

  return DNA_OK;
}

