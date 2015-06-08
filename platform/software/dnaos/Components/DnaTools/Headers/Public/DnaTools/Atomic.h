/****h* Tools/Atomic
 * SUMMARY
 * Atomic operation management.
 ****
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

#ifndef DNA_TOOLS_ATOMIC_H
#define DNA_TOOLS_ATOMIC_H

#include <stdint.h>
#include <stdbool.h>
#include <Processor/Processor.h>

/****f* Atomic/atomic_add
 * SUMMARY
 * Atomically add a value to a 32-bit data.
 *
 * SYNOPSIS
 */
int32_t atomic_add (int32_t * p_val, int32_t offset);


//static inline int32_t atomic_add (int32_t * p_val, int32_t offset)

/*
 * SOURCE
 */

/*{
  int32_t old_val = 0, new_val = 0, result = 0;

  do
  {
    old_val = *p_val;
    new_val = old_val + offset;
    result = cpu_compare_and_swap(p_val, old_val, new_val);
  }
  while (result != 0);

  return old_val;
}*/

/*
 ****/

#endif

