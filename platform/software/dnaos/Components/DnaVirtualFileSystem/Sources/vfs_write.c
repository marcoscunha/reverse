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

/****f* vfs/vfs_write
 * SUMMARY
 * Write data to a file.
 *
 * SYNOPSIS
 */

status_t vfs_write (int16_t fd, void * data, int32_t count, int32_t * p_ret)

/*
 * ARGUMENTS
 * * fd : the identifier of the file.
 * * buffer : the destination buffer.
 * * count : the number of bytes to write.
 *
 * FUNCTION
 * * Looks-up for the file corresponding to fd
 * * If it exists, then calls the file's write () function.
 *
 * RESULT
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
    DCACHE_INVAL(&file->offset, sizeof(uint64_t) );
    status = file -> vnode -> volume -> cmd -> write
      (file -> vnode -> volume -> data, file -> vnode -> data, file -> data,
       data, file -> offset, & n_data);

    check (write_error, status == DNA_OK, status);

    /*
     * If everything went well, increasing the file offset.
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& file -> lock);

    file -> offset += n_data;
    DCACHE_FLUSH(&file->offset,sizeof(uint64_t));

    lock_release (& file -> lock);
    cpu_trap_restore(it_status);

    /*
     * Release the file and return.
     */

    *p_ret = n_data;
    return file_put (fd);
  }

  rescue (write_error)
  {
    file_put (fd);
    *p_ret = -1;
    leave;
  }
}

/*
 ****/

