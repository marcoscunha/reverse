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

#include <Private/MemoryManager.h>
#include <DnaTools/DnaTools.h>

/****f* framework/memory_create
 * SUMMARY
 * Create the memory component.
 *
 * SYNOPSIS
 */

status_t memory_create (void)

/*
 * FUNCTION
 * - Initializes the BSS.
 * - Initializes each member of the memory manager element.
 * - Initializes the kernel allocator.
 *
 * SOURCE
 */

{
  uint32_t bss_size = 0, heap_size = 0;
  kernel_region_t region = NULL;

  /*
   * Initializes the BSS to 0
   */

  bss_size = (size_t) (CPU_BSS_END - CPU_BSS_START);
  dna_memset ((void *) CPU_BSS_START, 0, bss_size);

  /*
   * Initializes the kernel allocator
   */

  heap_size = OS_KERNEL_HEAP_SIZE / DNA_KERNEL_BLOCK_SIZE;

  region = (kernel_region_t) OS_KERNEL_HEAP_ADDRESS;
  region -> next = NULL;
  region -> prev = NULL;
  region -> nblocks = heap_size - 1;
  region -> status = DNA_KERNEL_FREE_REGION;

  kernel_allocator . next_free_region = region;
  kernel_allocator . next_created_region = NULL;
  kernel_allocator . nblocks = heap_size;
 // kernel_allocator . lock = 0;  
  lock_release(&kernel_allocator.lock);



  return DNA_OK;
}

/*
 ****/

