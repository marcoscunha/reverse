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
#include <Processor/Cache.h>

/****f* kernel/kernel_region_create
 * SUMMARY
 * Kernel memory allocation routine.
 *
 * SYNOPSIS
 */

status_t kernel_region_create (uint32_t required_nblocks,
    kernel_region_t * p_region)

/*
 * ARGUMENTS
 * * required_nblocks : the number of blocks to create
 *
 * FUNCTION
 * Worse-fit allocator.
 *
 * RESULT
 * If the allocation succeeded, a pointer to a valid region
 * of the memory, NULL otherwise.
 *
 * SOURCE
 */

{
  kernel_region_t current_region = kernel_allocator . next_free_region;
  kernel_region_t next_region = NULL, rptr = NULL, aptr = NULL;

  required_nblocks++;
  watch (status_t)
  {
    check (error_state, current_region != NULL, DNA_ERROR);
    check (error_state, p_region != NULL, DNA_ERROR);
    check (error_state, current_region -> nblocks > required_nblocks,
        DNA_OUT_OF_MEM);

    /*
     * Allocate the new region
     */

    if (current_region -> next != NULL) current_region -> next -> prev = NULL;
    kernel_allocator . next_free_region = current_region -> next;

    next_region = (kernel_region_t)((uint8_t *)current_region +
        (required_nblocks * DNA_KERNEL_BLOCK_SIZE));

    next_region -> status = DNA_KERNEL_FREE_REGION;
    DCACHE_FLUSH(&next_region->status,sizeof(int32_t));
    next_region -> nblocks = current_region -> nblocks - (required_nblocks);
    DCACHE_FLUSH(&next_region->nblocks,sizeof(int32_t));

    current_region -> prev = NULL;
    DCACHE_FLUSH(&current_region->prev,sizeof(kernel_region_t));
    current_region -> next = NULL;
    DCACHE_FLUSH(&current_region->next,sizeof(kernel_region_t));
    current_region -> status = DNA_KERNEL_ALLOCATED_REGION;
    DCACHE_FLUSH(&current_region->status,sizeof(uint32_t));
    current_region -> nblocks = required_nblocks;
    DCACHE_FLUSH(&current_region->nblocks, sizeof(uint32_t));

    /*
     * Re-order the free regions if necessary 
     */

    rptr = kernel_allocator . next_free_region;

    if (rptr == NULL)
    {
      kernel_allocator . next_free_region = next_region;
      next_region -> prev = NULL;
      next_region -> next = NULL;
    }
    else
    {
      DCACHE_INVAL(&rptr->next, sizeof(kernel_region_t));
      while (rptr -> next != NULL)
      {
        if (rptr -> nblocks <= next_region -> nblocks) break;
        rptr = rptr -> next;
      }

      if (rptr -> nblocks <= next_region -> nblocks)
      {
        next_region -> prev = rptr -> prev;

        rptr -> prev = next_region;
        
        next_region -> next = rptr;
        DCACHE_FLUSH(&next_region->next, sizeof(kernel_region_t));

        if (next_region -> prev != NULL)
        {
          next_region -> prev -> next = next_region;
        }
        else
        {
          kernel_allocator . next_free_region = next_region;
          DCACHE_FLUSH(&kernel_allocator . next_free_region, sizeof(kernel_region_t));
        }
      }
      else
      {
        next_region -> next = NULL;
        next_region -> prev = rptr;
        rptr -> next = next_region;
      }
    }

    /*
     * Add the allocated region to the list of created regions
     */

    aptr = kernel_allocator . next_created_region;

    if (aptr == NULL)
    {
      kernel_allocator . next_created_region = current_region;
      DCACHE_FLUSH(&kernel_allocator . next_created_region, sizeof(kernel_region_t));
    }
    else
    {
      DCACHE_INVAL(&aptr->next, sizeof(kernel_region_t));
      while (aptr -> next != NULL){
          aptr = aptr -> next;
          DCACHE_INVAL(&aptr->next, sizeof(kernel_region_t));
      }
      aptr -> next = current_region;
      DCACHE_FLUSH(&aptr->next,sizeof(kernel_region_t));
      current_region -> prev = aptr;
      DCACHE_FLUSH(&current_region->prev,sizeof(kernel_region_t));
    }

    *p_region = current_region;
    return DNA_OK;
  }

  rescue (error_state)
  {
    leave;
  }
}

/*
 ****/

