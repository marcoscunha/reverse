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

/****f* FilePrivate/file_release
 * SUMMARY
 * Release a file from the file pool.
 *
 * SYNOPSIS
 */

status_t file_put (int16_t fd)

/*
 * ARGUMENTS
 * * fd : the file descriptor.
 *
 * FUNCTION
 * Check if the fd entry exist in the current group pool. If it is the case,
 * decrements its usage counter, and delete the file when the counter reaches 0.
 *
 * RESULT
 * * DNA_INVALID_FD if fd is not a valid file
 * * DNA_OK if the operation succeed
 *
 * SOURCE
 */

{
  file_t file;
  int32_t tid;
  thread_info_t info;
  status_t status = DNA_OK;
  interrupt_status_t it_status = 0;
  vnode_t vnode = NULL;

  watch (status_t)
  {
    status = thread_find (NULL, & tid);
    ensure (status == DNA_OK, status);

    status = thread_get_info (tid, & info);
    ensure (status == DNA_OK, status);

    ensure (info . group >= 0, DNA_BAD_ARGUMENT);
    ensure (info . group < DNA_MAX_GROUP, DNA_BAD_ARGUMENT);

    /*
     * Look for the file in the pool.
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& file_pool . lock);

    file = file_pool . file[info . group][fd];
    check (error, file != NULL, DNA_INVALID_FD);
    check (error, file -> usage_counter > 0, DNA_ERROR);

    atomic_add (& file -> usage_counter, -1);
 
    if (file -> usage_counter == 0 && file -> destroy)
    {
      file_pool . file[info . group][fd] = NULL;

      lock_release (& file_pool . lock);
      cpu_trap_restore(it_status);

      status =  file -> vnode -> volume -> cmd -> free
        (file -> vnode -> volume -> data, file -> vnode -> data, file -> data);
      panic (status != DNA_OK);

      vnode = file -> vnode;
      kernel_free (file);

      status = vnode_put (vnode -> volume -> id, vnode -> id);
      panic (status != DNA_OK);
    }
    else
    {
      lock_release (& file_pool . lock);
      cpu_trap_restore(it_status);
    }

    dna_log(VERBOSE_LEVEL, "Put FD %d.", fd);

    return DNA_OK;
  }

  rescue (error)
  {
    lock_release (& file_pool . lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

