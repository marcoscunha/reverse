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

/****f* Operation/vfs_close
 * SUMMARY
 * Closes a file.
 *
 * SYNOPSIS
 */

status_t vfs_close (int16_t fd)

/*
 * ARGUMENTS
 * * fd : the file descriptor.
 *
 * FUNCTION
 * Close the file corresponding to the fd file descriptor.
 *
 * RESULT
 * * DNA_INVALID_FD if fd is not a valid file
 * * DNA_OK if the operation succeed
 *
 * SOURCE
 */

{
  file_t file = NULL;
  status_t status = DNA_OK;

  watch (status_t)
  {
    ensure (fd >= 0 && fd < DNA_MAX_FILE, DNA_INVALID_FD);

    /*
     * Get the file associated to the fd.
     */

    status = file_get (fd, & file);
    ensure (status == DNA_OK, status);

    /*
     * Close the file.
     */

    status = file -> vnode -> volume -> cmd -> close
      (file -> vnode -> volume -> data, file -> vnode -> data, file -> data);

    check (error, status == DNA_OK, status);

    /*
     * Release the file, destroy the file, and return.
     */

    status = file_put (fd);
    panic (status != DNA_OK);

    status = file_destroy (fd);
    panic (status != DNA_OK);

    return DNA_OK;
  }

  rescue (error)
  {
    status = file_put (fd);
    panic (status != DNA_OK);

    leave;
  }
}

/*
 ****/

