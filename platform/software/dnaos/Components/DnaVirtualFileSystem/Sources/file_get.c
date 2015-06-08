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

/****f* FilePrivate/file_acquire
 * SUMMARY
 * Acquire a file from the file pool.
 *
 * SYNOPSIS
 */

status_t file_get (int16_t fd, file_t * p_file)

/*
 * ARGUMENTS
 * * fd : a file descriptor
 * * p_file : a pointer to a file_t
 *
 * FUNCTION
 * Check if the fd entry exist in the current group pool. If it is the case, put
 * the file into *file after incrementing its usage counter.
 *
 * RESULT
 * * DNA_INVALID_FD if fd is not a valid file
 * * DNA_OK if the operation succeed
 *
 * SOURCE
 */

{
  int32_t tid;
  thread_info_t info;
  status_t status = DNA_OK;
  file_t file = NULL;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    status = thread_find (NULL, & tid);
    ensure (status == DNA_OK, status);

    status = thread_get_info (tid, & info);
    ensure (status == DNA_OK, status);

    ensure (info . group >= 0, DNA_BAD_ARGUMENT);
    ensure (info . group < DNA_MAX_GROUP, DNA_BAD_ARGUMENT);
    ensure (p_file != NULL, DNA_BAD_ARGUMENT);

    /*
     * Look for the file in the pool.
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& file_pool . lock);

    file = file_pool . file[info . group][fd];
    DCACHE_INVAL(&file->destroy, sizeof(bool));
    check (error, file != NULL && ! file -> destroy, DNA_INVALID_FD);

    atomic_add (& file -> usage_counter, 1);

    lock_release (& file_pool . lock);
    cpu_trap_restore(it_status);

    dna_log(VERBOSE_LEVEL, "Got FD %d.", fd);

    *p_file = file;
    DCACHE_FLUSH(p_file, sizeof(file_t));
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

