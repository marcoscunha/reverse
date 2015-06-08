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

/****f* kernel/kernel_region_destroy
 * SUMMARY
 * Decreate a previously created kernel memory.
 *
 * SYNOPSIS
 */

status_t kernel_region_destroy (kernel_region_t region)

/*
 * ARGUMENTS
 * * region : the region to destroy
 *
 * FUNCTION
 * Link the region with the others, decreasing-size ordered.
 *
 * RESULT
 * Always return DNA_OK.
 *
 * SOURCE
 */

{
  kernel_region_t ptr = NULL;
  uint32_t vptr = 0, vregion = 0;
  uint32_t vrsize = 0, vpsize = 0;

  /*
   * Remove the region from the list of created regions
   */

  if (region -> next != NULL)
  {
    region -> next -> prev = region -> prev;
    region -> next = NULL;
    DCACHE_FLUSH(&region->next->prev, sizeof(kernel_region_t));
    DCACHE_FLUSH(&region->next, sizeof(kernel_region_t));
  }

  if (region -> prev != NULL)
  {
    region -> prev -> next = region -> next;
    region -> prev = NULL;
    DCACHE_FLUSH(&region->prev->next, sizeof(kernel_region_t));
    DCACHE_FLUSH(&region->prev, sizeof(kernel_region_t));
  }
  else
  {
    kernel_allocator . next_created_region = region -> next;
  }


  /*
   * Add the region as a free region
   */

  ptr = kernel_allocator . next_free_region;

  if (ptr == NULL) kernel_allocator . next_free_region = region;
  else {
    vregion = (uint32_t)region;
    vrsize = region -> nblocks * DNA_KERNEL_BLOCK_SIZE;

    /*
     * First, we increase the new region size
     * with connexes regions
     */

    while (ptr != NULL) {
      vptr = (uint32_t)ptr;
      vpsize = ptr -> nblocks * DNA_KERNEL_BLOCK_SIZE;

      if (vptr + vpsize == vregion) {
        ptr -> nblocks += region -> nblocks;
        region = ptr;

        if (ptr -> prev != NULL) ptr -> prev -> next = ptr -> next;
        else kernel_allocator . next_free_region = ptr -> next;
        if (ptr -> next != NULL) ptr -> next -> prev = ptr -> prev;
      }
      else if (vregion + vrsize == vptr) {
        region -> nblocks += ptr -> nblocks;
        DCACHE_FLUSH(&region->nblocks,sizeof(uint32_t));

        if (ptr -> prev != NULL) ptr -> prev -> next = ptr -> next;
        else kernel_allocator . next_free_region = ptr -> next;
        if (ptr -> next != NULL) ptr -> next -> prev = ptr -> prev;
      }

      ptr = ptr -> next;
    }

    /*
     * Then, we order it in term of worst fit.
     * This way, alloc only take the first in the list.
     */

    ptr = kernel_allocator . next_free_region;
    if (ptr == NULL) {
      region -> prev = NULL;
      region -> next = NULL;
      kernel_allocator . next_free_region = region;
    }
    else {
      while (ptr -> next != NULL) {
        if (ptr -> nblocks <= region -> nblocks)  break;
        ptr = ptr -> next;
      }
      
      if (ptr -> nblocks <= region -> nblocks) {
        region -> prev = ptr -> prev;
        region -> next = ptr;

        if (ptr -> prev != NULL) ptr -> prev -> next = region;
        else kernel_allocator . next_free_region = region;
        ptr -> prev = region;
      }
      else {
        ptr -> next = region;
        region -> prev = ptr;
        region -> next = NULL;
        DCACHE_FLUSH(region,sizeof(struct _kernel_region));
      }
    }
  }  

  return DNA_OK;
}

/*
 ****/

