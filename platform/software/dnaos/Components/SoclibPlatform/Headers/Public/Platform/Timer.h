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

#ifndef PLATFORM_TIMER_H
#define PLATFORM_TIMER_H

#include <stdint.h>

#define PLATFORM_TIMER_RES 10

typedef struct _soclib_timer_port
{
  uint32_t value;
  uint32_t mode;
  uint32_t period;
  uint32_t irq_ack;
}
* soclib_timer_port_t;

extern soclib_timer_port_t PLATFORM_TIMER_BASE;

#endif

