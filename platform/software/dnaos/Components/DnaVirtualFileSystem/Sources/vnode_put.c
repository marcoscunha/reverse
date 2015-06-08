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
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* VNode/vnode_put
 * SUMMARY
 * Put a vnode back.
 *
 * SYNOPSIS
 */

status_t vnode_put (int32_t vid, int64_t vnid)

/*
 * ARGUMENTS
 * * vid : a volume ID.
 * * vnid : the vnode ID to put back.
 *
 * RESULT
 * * DNA_NO_VOLUME if the volume corresponding to vid does not exist.
 * * DNA_NO_VNODE if the vnode corresponding to vnid does not exist.
 * * DNA_OK if the operation succeeded.
 * * DNA_ERROR if an error occured.
 *
 * SOURCE
 */

{
  vnode_t vnode = NULL;
  volume_t volume = NULL;
  void * data = NULL;
  bool destroy = false;

  status_t status = DNA_OK;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& vnode_manager . vnode_list . lock);

    vnode = queue_lookup (& vnode_manager . vnode_list,
        vnode_id_inspector, vnid, vid);

    check (invalid_vnode, vnode != NULL, DNA_NO_VNODE);

    atomic_add (& vnode -> usage_counter, -1);

    if (vnode -> usage_counter == 0)
    {
      queue_extract (& vnode_manager . vnode_list, & vnode -> link);

      lock_release (& vnode_manager . vnode_list . lock);
      cpu_trap_restore(it_status);

      volume = vnode -> volume;
      data = vnode -> data;
      destroy = vnode -> destroy;
      status = volume -> cmd -> write_vnode (volume -> data, data);
      ensure (status == DNA_OK, status);

      if (destroy)
      {
        status = volume -> cmd -> destroy_vnode (volume -> data, data); 
        ensure (status == DNA_OK, status);
      }

      kernel_free (vnode);
    }
    else
    {
      lock_release (& vnode_manager . vnode_list . lock);
      cpu_trap_restore(it_status);
    }

    return DNA_OK;
  }

  rescue (invalid_vnode)
  {
    lock_release (& vnode_manager . vnode_list . lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

