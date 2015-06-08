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

#ifndef PLATFORM_AICU_H
#define PLATFORM_AICU_H

#include <stdint.h>

#define SOCLIB_AICU_MAX_SLOTS 32

typedef struct _soclib_aicu_slot
{
  uint32_t status;
  uint32_t mask;
  uint32_t active_handler;
  uint32_t active_id;
}
soclib_aicu_slot_t;

typedef struct _soclib_aicu_port
{
  uint32_t control;
  uint32_t reserved_0[3];

  uint32_t handlers[SOCLIB_AICU_MAX_SLOTS];
  uint32_t reserved_1[64 - (SOCLIB_AICU_MAX_SLOTS + 4)];

  soclib_aicu_slot_t slot[SOCLIB_AICU_MAX_SLOTS];
}
* soclib_aicu_port_t;

extern soclib_aicu_port_t PLATFORM_AICU_BASE;

#endif

