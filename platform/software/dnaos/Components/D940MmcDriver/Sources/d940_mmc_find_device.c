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
#include <Private/Driver.h>
#include <DnaTools/DnaTools.h>

device_cmd_t d940_mmc_commands =
{
  d940_mmc_open,
  d940_mmc_close,
  d940_mmc_free,
  d940_mmc_read,
  d940_mmc_write,
  d940_mmc_control
};

device_cmd_t * d940_mmc_find_device (const char * name)
{
  return & d940_mmc_commands;
}

