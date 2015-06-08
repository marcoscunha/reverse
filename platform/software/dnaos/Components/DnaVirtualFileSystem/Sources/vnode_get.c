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
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* VNode/vnode_get
 * SUMMARY
 * Get a vnode from a volume.
 *
 * SYNOPSIS
 */

status_t vnode_get (int32_t vid, int64_t vnid, void ** data)

/*
 * ARGUMENTS
 * * vid : the id of the volume to get the vnode from.
 * * vnid : the vnode to get.
 * * data : a pointer to the shadow vnode.
 *
 * RESULT
 * * DNA_ERROR if an error occured.
 * * DNA_NO_VOLUME if the volume corresponding to vid does not exist.
 * * DNA_OUT_OF_MEM if the system ran out of memory during the operation.
 * * DNA_OK if the operation succeeded.
 *
 * SOURCE
 */

{
  vnode_t vnode = NULL;
  volume_t volume = NULL;
  status_t status = DNA_OK;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (data != NULL, DNA_ERROR);

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& vnode_manager . vnode_list . lock);

    vnode = queue_lookup (& vnode_manager . vnode_list,
        vnode_id_inspector, vnid, vid);

    lock_release (& vnode_manager . vnode_list . lock);
    cpu_trap_restore(it_status);

    if (vnode == NULL)
    {
      it_status = cpu_trap_mask_and_backup();
      lock_acquire (& volume_manager . volume_list . lock);

      volume = queue_lookup (& volume_manager . volume_list,
          volume_id_inspector, vid);

      lock_release (& volume_manager . volume_list . lock);
      cpu_trap_restore(it_status);

      status = volume -> cmd -> read_vnode (volume -> data, vnid, data);
      ensure (status == DNA_OK, status);

      status = vnode_create (vnid, volume -> id, *data);
      ensure (status == DNA_OK, status);
    }
    else if (! vnode -> destroy)
    {
      *data = vnode -> data;
      atomic_add (& vnode -> usage_counter, 1);
    }

    return DNA_OK;
  }
}

/*
 ****/

