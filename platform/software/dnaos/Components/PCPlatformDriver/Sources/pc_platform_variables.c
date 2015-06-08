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

driver_t pc_platform_module = {
  "pc_platform",
  pc_platform_init_hardware,
  pc_platform_init_driver,
  pc_platform_uninit_driver,
  pc_platform_publish_devices,
  pc_platform_find_device
};

char ** pc_platform_devices = NULL;

device_cmd_t pc_tty_commands =
{
  pc_tty_open,
  pc_tty_close,
  pc_tty_free,
  pc_tty_read,
  pc_tty_write,
  pc_tty_control
};

