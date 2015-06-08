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

/****f* Operation/vfs_open
* SUMMARY
 * Opens a file.
 *
 * SYNOPSIS
 */

status_t vfs_open (char * restrict path, int32_t mode,
    int32_t perms, int16_t * p_fd)

/*
 * ARGUMENTS
 * * path : path of the file to open.
 * * flags : ignored.
 * * mode : ignored.
 * * p_fd : a pointer to a file descriptor
 *
 * FUNCTION
 * Parses the path and checks if it is valid. Then checks if the file shoulb be
 * created or opened. Finally, calls the appropriate function and returns.
 *
 * RESULT
 * * DNA_NO_VNODE : the requested file does not exist
 * * DNA_ERROR : error while parsing the file path
 * * DNA_OK : the operation succeeded
 *
 * SOURCE
 */

{
  int16_t fd = -1;
  file_t file = NULL;
  int64_t vnid = -1, file_vnid = -1;
  vnode_t vnode = NULL;
  volume_t volume = NULL;
  void * data = NULL, * file_data = NULL;
  char buffer[DNA_PATH_LENGTH], token[DNA_FILENAME_LENGTH];
  interrupt_status_t it_status = 0;
  status_t status = DNA_OK;

  watch (status_t)
  {
    ensure (path != NULL && p_fd != NULL, DNA_ERROR);
    dna_strcpy (buffer, path);

    if ((mode & DNA_CREATE) != 0)
    {
      /*
       * Spilt the path into base + token
       */

      status = path_split (buffer, token);
      ensure (status == DNA_OK, status);

      /*
       * Get the vnode corresponding to the base directory
       */

      status = vnode_walk (buffer, & volume, & vnid, & data);
      ensure (status == DNA_OK, status);

      /*
       * Ask to create the file.
       */

      status = volume -> cmd -> create (volume -> data, data, token,
          mode, perms, & file_vnid, & file_data);
      ensure (status == DNA_OK, status);

      /*
       * Release the parent file, it is not required anymore.
       */

      status = vnode_put (volume -> id, vnid);
      panic (status != DNA_OK);
    }
    else
    {
      /*
       * Get the vnode corresponding to the base directory
       */

      status = vnode_walk (buffer, & volume, & vnid, & data);
      ensure (status == DNA_OK, status);

      /*
       * Ask to open the file
       */

      status = volume -> cmd -> open (volume -> data, data, mode, & file_data);
      ensure (status == DNA_OK, status);
    }
  
    /*
     * Find the new vnode.
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& vnode_manager . lock);

    vnode = queue_lookup (& vnode_manager . vnode_list,
        vnode_id_inspector, vnid, volume -> id);

    lock_release (& vnode_manager . lock);
    cpu_trap_restore (it_status);

    ensure (vnode != NULL, DNA_NO_VNODE);

    /*
     * Acquire a new file.
     */

    status = file_create (vnode, mode, file_data, & fd, & file); 
    ensure (status == DNA_OK, status);
 
    *p_fd = fd;
    return DNA_OK;
  }
}

/*
 ****/

