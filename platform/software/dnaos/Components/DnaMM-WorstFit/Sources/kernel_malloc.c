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

/**** kernel/kernel_malloc
 * SUMMARY
 * Fast malloc for the kernel.
 *
 * SYNOPSIS
 */

void * kernel_malloc (uint32_t size, bool erase)

/*
 * ARGUMENTS
 * * size : the amount of memory to create, in bytes
 *
 * RESULT
 * * A valid data pointer in case of success.
 * * NULL in case of failure.
 *
 * SOURCE
 */

{
  uint8_t * area = NULL;
  kernel_region_t region = NULL;
  uint32_t nblocks = 0;
  status_t status = DNA_OK;
  interrupt_status_t it_status = 0;

  watch (void *)
  {
    check (invalid_argument, size != 0, NULL);

    nblocks = size / DNA_KERNEL_BLOCK_SIZE;
    nblocks += (((size - nblocks * DNA_KERNEL_BLOCK_SIZE) != 0) ? 1 : 0) + 1;



    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& kernel_allocator . lock);

    status = kernel_region_create (nblocks, & region);
    check (create_failed, status == DNA_OK, NULL);   

    kernel_allocator . nblocks -= region -> nblocks;
    DCACHE_FLUSH(&kernel_allocator.nblocks, sizeof(uint32_t));

    area = (uint8_t *)(region) + DNA_KERNEL_BLOCK_SIZE;

    lock_release (& kernel_allocator . lock);
    cpu_trap_restore(it_status);
#ifdef WRITEBACK
    if (erase) dna_memset (area, 0,
        DNA_KERNEL_BLOCK_SIZE * (region -> nblocks - 2));
#else
    if (erase) dna_memset (area, 0,
        DNA_KERNEL_BLOCK_SIZE * (region -> nblocks - 1));
#endif

    return (void *)area;
  }

  rescue (create_failed)
  {
    lock_release (& kernel_allocator . lock);
    cpu_trap_restore(it_status);
  }

  rescue (invalid_argument)
  {
    leave;
  }
}

/*
 * NOTE
 * Since we are in the kernel, the malloc are ALWAYS zeroized.
 ****/

