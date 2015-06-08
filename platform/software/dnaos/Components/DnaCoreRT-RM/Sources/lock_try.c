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

#include <stdint.h>
#include <Private/Core.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* Lock/lock_try
 * SUMMARY
 * Try to acquire a spinlock
 *
 * SYNOPSIS
 */

status_t lock_try (spinlock_t * lock, bool acquire)

/*
 * ARGUMENTS
 * * lock : a pointer to a lock
 * * acquire : if acquire, then acquire the lock if it's free
 *
 * FUNCTION
 * Try to lock a spin lock using the cpu_test_and_set function from the HAL.
 *
 * RETURN
 * * DNA_WOULD_BLOCK: the lock is locked and acquire is false
 * * DNA_OK: the lock has been taken
 *
 * SOURCE
 */

{
  status_t status = DNA_OK;

  if (cpu_test_and_set (lock) == 1)
  {
    if (acquire) status = DNA_ERROR;
    else status = DNA_WOULD_BLOCK;
  }
  else 
  {
    if (! acquire) *lock = 0;
    status = DNA_OK;
  }

  cpu_cache_sync ();
  return status;
}

/*
 ****/

