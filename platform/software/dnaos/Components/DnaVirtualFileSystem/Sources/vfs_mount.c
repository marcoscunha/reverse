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

/****f* Operation/vfs_mount
 * SUMMARY
 * Mounts a volume.
 *
 * SYNOPSIS
 */

status_t vfs_mount (char * restrict source, char * restrict target,
    char * restrict fs_type, uint32_t flags, void * data)

/*
 * ARGUMENTS
 * * source : the path of the volume.
 * * destination : the path of the mount point.
 * * fs_type : the type of the filesystem to use.
 * * flags : ignored.
 * * data : ignored.
 *
 * FUNCTION
 * * Allocates a new vnode_t element.
 * * Looks for the filesystem corresponding to fs_type.
 * * If it exists, associates it to the vnode_t element.
 * * Add the new vnode into the vnode list of the VFS manager.
 *
 * RESULT
 * DNA_OK : the operation succeeded.
 * DNA_OUT_OF_MEM : the memory allocation failed.
 * DNA_ERROR : fs_type doesn't exist.
 *
 * SOURCE
 */

{
  filesystem_t * fs = NULL;
  int64_t host_vnid = -1;
  volume_t host_volume = NULL, volume = NULL;

  void * node_data = NULL;
  status_t status = DNA_OK;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (source != NULL && target != NULL
        && fs_type != NULL, DNA_ERROR);

    /*
     * Find the vnode corresponding to the path
     */

    status = vnode_walk (target, & host_volume, & host_vnid, & node_data);
    ensure (status == DNA_OK || status == DNA_NO_VOLUME, status);

    /*
     * Check if the vnode is not already associated
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& volume_manager . volume_list . lock);

    volume = queue_lookup (& volume_manager . volume_list,
        volume_host_inspector, host_volume, host_vnid);

    lock_release (& volume_manager . volume_list . lock);
    cpu_trap_restore (it_status);

    ensure (volume == NULL, DNA_VNODE_MOUNTED);

    /*
     * Get the required filesystem
     */

    status = filesystem_load (fs_type, & fs);
    ensure (status == DNA_OK, status);

    /*
     * Create the volume
     */

    status = volume_create (host_volume, host_vnid, fs -> cmd, & volume);
    ensure (volume != NULL, DNA_OUT_OF_MEM);

    /*
     * Mount the filesystem
     */

    status = fs -> cmd -> mount (volume -> id, source, flags, data,
        & volume -> data, & volume -> root_vnid);

    check (cannot_mount, status == DNA_OK, status);

    /*
     * Put the host vnode back
     */

    if (host_volume != NULL)
    {
      status = vnode_put (host_volume -> id, host_vnid);
      panic (status != DNA_OK);
    }

    return DNA_OK;
  }

  rescue (cannot_mount)
  {
    status = volume_destroy (volume);
    panic (status != DNA_OK);

    leave;
  }
}

/*
 ****/

