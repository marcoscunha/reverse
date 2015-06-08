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
#include <Private/MemoryManager.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* kernel/kernel_free
 * SUMMARY
 * Free previously created kernel memory.
 *
 * SYNOPSIS
 */

status_t kernel_free (void * area)

/*
 * ARGUMENTS
 * * area : a pointer to the area to destroy
 *
 * FUNCTION
 * Destroy the area and register it as available.
 *
 * RESULT
 * * DNA_ERROR if the argument area is NULL.
 * * DNA_OK if the operation succeeded.
 *
 * SOURCE
 */

{
  status_t status = DNA_OK;
  kernel_region_t region = NULL;
  int32_t region_nblocks;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    check (invalid_argument, area != NULL, DNA_ERROR);

    region = (kernel_region_t)((uint32_t)area - DNA_KERNEL_BLOCK_SIZE);
    region -> status = DNA_KERNEL_FREE_REGION;
    DCACHE_FLUSH(&region->status,sizeof(uint32_t));
    region_nblocks = region -> nblocks;

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& kernel_allocator . lock);

    status = kernel_region_destroy (region);
    if (status == DNA_OK) kernel_allocator . nblocks += region_nblocks;

    lock_release (& kernel_allocator . lock);
    cpu_trap_restore(it_status);

    return status;
  }

  rescue (invalid_argument)
  {
    leave;
  }
}

/*
 ****/

