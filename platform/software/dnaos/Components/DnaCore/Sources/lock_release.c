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

/****f* Lock/lock_release
 * SUMMARY
 * Unlock a spin lock.
 *
 * SYNOPSIS
 */

void lock_release (spinlock_t * lock)

/*
 * ARGUMENTS
 * * lock : a pointer to a lock
 *
 * FUNCTION
 * Set the lock value to 0.
 *
 * SOURCE
 */

{
#ifdef WRITEBACK
  if((uintptr_t)lock & (uintptr_t)0x1F){
      dna_printf("Lock not aligned %p 0x%08x\n",lock,(uintptr_t)lock & (uintptr_t)0x1F );
  }
#endif
  *lock = 0; 
  DCACHE_FLUSH_FAST(lock); // Dangerours when it is not aligned !
  cpu_cache_sync ();
}

/*
 ****/

