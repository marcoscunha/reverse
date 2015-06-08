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
#include <Processor/Processor.h>

/****f* Operation/vfs_umount
 * SUMMARY
 * Unmount a volume.
 *
 * SYNOPSIS
 */

status_t vfs_umount (char * restrict target)

/*
 * ARGUMENTS
 * * target : the target mount point where the volume is mounted.
 *
 * RESULT
 *
 * SOURCE
 */

{
  int64_t host_vnid = -1;
  volume_t host_volume = NULL, volume = NULL;
  vnode_t vnode = NULL;
  void * node_data = NULL;
  status_t status = DNA_OK;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    /*
     * Find the vnode corresponding to the path
     */

    status = vnode_walk (target, & host_volume, & host_vnid, & node_data);
    ensure (status == DNA_OK || status == DNA_NO_VOLUME, status);
    ensure (host_volume != NULL && host_vnid != -1, DNA_NO_VNODE);

    /*
     * Check if something is mounted on this node
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& volume_manager . volume_list . lock);

    volume = queue_lookup (& volume_manager . volume_list,
        volume_host_inspector, host_volume, host_vnid);

    lock_release (& volume_manager . volume_list . lock);
    cpu_trap_restore (it_status);

    ensure (volume != NULL, DNA_ERROR);

    /*
     * Check if the volume is being used
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& vnode_manager . vnode_list . lock);

    vnode = queue_lookup (& vnode_manager . vnode_list,
        vnode_volume_inspector, volume);

    lock_release (& vnode_manager . vnode_list . lock);
    cpu_trap_restore (it_status);

    ensure (vnode != NULL, DNA_VOLUME_IN_USE);

    /*
     * Destroy the volume
     */

    return volume_destroy (volume);
  }
}

/*
 ****/
