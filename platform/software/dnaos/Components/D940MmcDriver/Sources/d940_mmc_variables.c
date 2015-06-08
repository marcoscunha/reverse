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
#include <Private/MMC.h>
#include <DnaTools/DnaTools.h>

mmc_driver_t d940_mmc_driver = { .card = NULL };

driver_t d940_mmc_module =
{
  "D940MmcDriver",
  d940_mmc_init_hardware,
  d940_mmc_init_driver,
  d940_mmc_uninit_driver,
  d940_mmc_publish_devices,
  d940_mmc_find_device
};

const char * d940_mmc_devices[] =
{
  "disk/mmc/0/raw",
  NULL
};

