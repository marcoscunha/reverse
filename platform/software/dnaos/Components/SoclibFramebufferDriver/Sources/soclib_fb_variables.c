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

soclib_framebuffer_t * FB = NULL;
char ** soclib_fb_devices = NULL;

device_cmd_t soclib_fb_commands =
{
  soclib_fb_open,
  soclib_fb_close,
  soclib_fb_free,
  soclib_fb_read,
  soclib_fb_write,
  soclib_fb_control
};

driver_t soclib_fb_module = {
  "soclib_fb",
  soclib_fb_init_hardware,
  soclib_fb_init_driver,
  soclib_fb_uninit_driver,
  soclib_fb_publish_devices,
  soclib_fb_find_device
};

