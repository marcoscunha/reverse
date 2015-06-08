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

#include <Private/Core.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* Lock/lock_acquire
 * SUMMARY
 * Acquire a spin lock.
 *
 * SYNOPSIS
 */

void lock_acquire (spinlock_t * lock)

/*
 * ARGUMENTS
 * * lock : a pointer to a lock
 *
 * FUNCTION
 * Lock a spin lock using the cpu_test_and_set function from the HAL.
 *
 * SOURCE
 */

{
  while (cpu_test_and_set (lock) == 1);
  cpu_cache_sync ();
}

/*
 ****/

