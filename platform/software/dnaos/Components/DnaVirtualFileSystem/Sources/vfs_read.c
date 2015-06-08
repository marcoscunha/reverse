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

/****f* Operation/vfs_read
 * SUMMARY
 * Read data from a file.
 *
 * SYNOPSIS
 */

status_t vfs_read (int16_t fd, void * data, int32_t count, int32_t * p_ret)

/*
 * ARGUMENTS
 * * fd : the identifier of the file.
 * * buffer : the destination buffer.
 * * count : the number of bytes to read.
 * * p_ret : a pointer to the return value
 *
 * FUNCTION
 * Looks-up for the file corresponding to fd. If it exists, then calls the
 * file's read () function.
 *
 * RESULT
 * * DNA_ERROR : an error occured while acquiring or releasing the file
 * * DNA_OK : the operation succeeded
 *
 * SOURCE
 */

{
  file_t file = NULL;
  int32_t n_data = count;
  interrupt_status_t it_status;
  status_t status = DNA_OK;

  watch (status_t)
  {
    ensure (data != NULL && p_ret != NULL && count > 0, DNA_ERROR);
    ensure (fd >= 0 && fd < DNA_MAX_FILE, DNA_INVALID_FD);

    /*
     * Get the file associated to the fd.
     */

    status = file_get (fd, & file);
    ensure (status == DNA_OK, status);

    /*
     * Read the file.
     */

    status = file -> vnode -> volume -> cmd -> read
      (file -> vnode -> volume -> data, file -> vnode -> data, file -> data,
       data, file -> offset, & n_data);

    check (read_error, status == DNA_OK, status);

    /*
     * If everything went well, increasing the file offset.
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& file -> lock);

    file -> offset += n_data;

    lock_release (& file -> lock);
    cpu_trap_restore(it_status);

    /*
     * Release the file and return.
     */

    status = file_put (fd);
    panic (status != DNA_OK);

    *p_ret = n_data;
    return DNA_OK;
  }

  rescue (read_error)
  {
    status = file_put (fd);
    panic (status != DNA_OK);

    *p_ret = -1;
    leave;
  }
}

/*
 ****/
