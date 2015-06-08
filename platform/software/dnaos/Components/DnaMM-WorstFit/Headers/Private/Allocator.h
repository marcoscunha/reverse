/****h* MemoryManagerPrivate/AllocatorPrivate
 * SUMMARY
 * Base functions for the kernel allocator.
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

#ifndef DNA_MEMORYMANAGER_ALLOCATOR_PRIVATE_H
#define DNA_MEMORYMANAGER_ALLOCATOR_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>
#include <Core/Core.h>
#include <DnaTools/Status.h>

/****d* AllocatorPrivate/DNA_KERNEL_BLOCK_SIZE
 * SUMMARY
 * Defines the minimum size of a memory block.
 *
 * SOURCE
 */

#define DNA_KERNEL_BLOCK_SIZE (2 * sizeof (struct _kernel_region))

/*
 ****/

/****v* AllocatorPrivate/region_status
 * SUMMARY
 * Status of region.
 *
 * SOURCE
 */
#ifdef WRITEBACK
typedef uint32_t region_status;
#define  DNA_KERNEL_FREE_REGION       0xBEEF
#define  DNA_KERNEL_ALLOCATED_REGION  0xDEAD
#else
enum region_status
{
  DNA_KERNEL_FREE_REGION = 0xBEEF,
  DNA_KERNEL_ALLOCATED_REGION = 0xDEAD
};
#endif
/*
 ****/

/****t* AllocatorPrivate/kernel_region_t
 * SUMMARY
 * Type of an allocated kernel memory region.
 *
 * SOURCE
 */

typedef struct _kernel_region
{
  int32_t nblocks;
  int32_t status;

  struct _kernel_region * next;
  struct _kernel_region * prev;
}
* kernel_region_t;

/*
 ****/

/****t* AllocatorPrivate/kernel_allocator_t
 * SUMMARY
 * Type of the kernel allocator.
 *
 * SOURCE
 */

typedef struct _kernel_allocator
{
  int32_t nblocks;
  kernel_region_t next_created_region;
  kernel_region_t next_free_region;
  #ifdef WRITEBACK
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare[7];
  #else
  spinlock_t lock __attribute__((aligned(32)));
  #endif

}
kernel_allocator_t;

/*
 ****/

extern uint32_t OS_KERNEL_HEAP_ADDRESS;
extern uint32_t OS_KERNEL_HEAP_SIZE;

extern kernel_allocator_t kernel_allocator;

extern void * kernel_malloc (uint32_t size, bool erase);
extern status_t kernel_free (void * area);

extern status_t kernel_region_create (uint32_t required_nblocks,
    kernel_region_t * p_region);
extern status_t kernel_region_destroy (kernel_region_t region);

#endif
