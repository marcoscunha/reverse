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

status_t soclib_platform_init_hardware (void)
{
  soclib_timer_port_t timer;

  cpu_write (UINT32, & (PLATFORM_AICU_BASE -> control), 0x1);

  for (int32_t i = 0; i < cpu_mp_count (); i += 1)
  {
    timer = & PLATFORM_TIMER_BASE[i];
    cpu_write(UINT32, & (timer -> mode), 1); 
    cpu_write (UINT32, & (PLATFORM_AICU_BASE -> slot[i] . mask), 0x3);
  }

  return DNA_OK;
}

