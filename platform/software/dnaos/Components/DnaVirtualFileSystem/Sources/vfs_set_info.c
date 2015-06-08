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

/****f* Operation/vfs_set_info
 * SUMMARY
 * Get the information of a given file.
 *
 * SYNOPSIS
 */

status_t vfs_set_info (int16_t fd, file_info_t info, int32_t mask)

/*
 * ARGUMENTS
 * * fd : the file identifier
 * * p_info : a pointer to a file_info_t structure
 *
 * FUNCTION
 * Looks-up for the file corresponding to fd * If it exists, then calls the
 * file's set_info() function.
 *
 * RESULT
 * File attributes are modified depending on the mask.
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
     * Call ioctl on the file.
     */

    status = file -> vnode -> volume -> cmd -> set_info
      (file -> vnode -> volume -> data, file -> vnode -> data,
       file -> data, info, mask);

    check (error, status == DNA_OK, status);

    /*
     * Release the file and return.
     */

    status = file_put (fd);
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

