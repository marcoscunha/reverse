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

#include <Private/Ethernet.h>

device_cmd_t d940_ethernet_commands =
{
  d940_ethernet_open,
  d940_ethernet_close,
  NULL,
  d940_ethernet_read,
  d940_ethernet_write,
  d940_ethernet_control
};

device_cmd_t * d940_ethernet_find_device (const char * name)
{
  if(dna_strcmp (name, "ethernet/d940/0") == 0)  
    return & d940_ethernet_commands;

  return NULL;
}


