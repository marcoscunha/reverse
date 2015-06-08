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

#include <Private/VirtualFileSystem.h>
#include <DnaTools/DnaTools.h>
#include <MemoryManager/MemoryManager.h>

/****f* VNode/vnode_create
 * SUMMARY
 * Create a vnode.
 *
 * SYNOPSIS
 */

status_t vnode_create (int64_t vnid, int32_t vid, void * data)

/*
 * ARGUMENTS
 * * vnode : a pointer to a vnode_t;
 *
 * RESULT
 * * DNA_OK if the operation succeeded.
 * * DNA_OUT_OF_MEM if the system ran out of memory during the operation.
 *
 * SOURCE
 */

{
  vnode_t vnode = NULL;
  volume_t volume = NULL;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    /*
     * We first volume corresponding to vid
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& volume_manager . volume_list . lock);

    volume = queue_lookup (& volume_manager . volume_list,
        volume_id_inspector, vid);

    lock_release (& volume_manager . volume_list . lock);
    cpu_trap_restore (it_status);

    ensure (volume != NULL, DNA_NO_VOLUME);
    
    /*
     * Then we can check if the vnid is not already taken
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& vnode_manager . vnode_list . lock);

    vnode = queue_lookup (& vnode_manager . vnode_list,
        vnode_id_inspector, vnid, vid);

    lock_release (& vnode_manager . vnode_list . lock);
    cpu_trap_restore(it_status);

    ensure (vnode == NULL, DNA_ERROR);

    /*
     * Next, we create the vnode
     */
#if 1
    {
    struct _vnode* kalloc_addr;
    kalloc_addr = kernel_malloc (sizeof (struct _vnode)+0x1F, true);
    vnode = (struct _vnode*)(((uintptr_t)kalloc_addr+0x1F) & ~(uintptr_t)0x1F);
    vnode->kalloc_addr = kalloc_addr;
    }
#else
    vnode = kernel_malloc (sizeof (struct _vnode), true);
#endif
    ensure (vnode != NULL, DNA_OUT_OF_MEM);

    vnode -> id = vnid;
    DCACHE_FLUSH(&vnode->id, sizeof(uint64_t)); 
    vnode -> volume = volume;
    DCACHE_FLUSH(&vnode->volume, sizeof(volume_t*)); 
    vnode -> destroy = false;
    DCACHE_FLUSH(&vnode->destroy, sizeof(bool)); 
    vnode -> data = data;
    DCACHE_FLUSH(&vnode->data, sizeof(void*)); 
    vnode -> usage_counter = 1;
    DCACHE_FLUSH(&vnode->usage_counter, sizeof(uint32_t)); 
    /*
     * And finally we add it to the vnode manager
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& vnode_manager . vnode_list . lock);

    queue_add (& vnode_manager . vnode_list, vnode);

    lock_release (& vnode_manager . vnode_list . lock);
    cpu_trap_restore(it_status);

    return DNA_OK;
  }
}

/*
 ****/

