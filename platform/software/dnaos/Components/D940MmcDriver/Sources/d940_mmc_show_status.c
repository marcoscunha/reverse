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

#include <Private/Status.h>
#include <DnaTools/DnaTools.h>

static char * error_string[] = 
{
  "response index error",
  "response direction error",
  "response CRC error",
  "response end bit error",
  "response time-out error",
  "data CRC error",
  "data time-out error",
  "overrun",
  "underrun",
  NULL
};

static uint32_t error_code[] =
{
  0x00010000,
  0x00020000,
  0x00040000,
  0x00080000,
  0x00100000,
  0x00200000,
  0x00400000,
  0x40000000,
  0x80000000,
};

void d940_mmc_show_status (d940_mmc_status_t status)
{
  if ((status . raw & STATUS_ERROR_MASK) != 0)
  {
    for (int32_t index = 0; error_string[index] != NULL; index += 1)
    {
      if ((status . raw & error_code[index]) != 0)
      {
        log (INFO_LEVEL, "%s", error_string[index]);
      }
    }
  }
}

