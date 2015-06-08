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
#include <Processor/Cache.h>


/****f* FilePrivate/file_acquire
 * SUMMARY
 * Create a file in the file pool.
 *
 * SYNOPSIS
 */

status_t file_create (vnode_t vnode, int32_t mode, void * data,
    int16_t * p_fd, file_t * p_file)

/*
 * ARGUMENTS
 * * p_fd : a pointer to a file descriptor
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
  int16_t fd = 0;
  int32_t tid;
  thread_info_t info;
  status_t status = DNA_OK;
  file_t file = NULL, * file_array = NULL;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    status = thread_find (NULL, & tid);
    ensure (status == DNA_OK, status);

    status = thread_get_info (tid, & info);
    ensure (status == DNA_OK, status);

    ensure (info . group >= 0, DNA_BAD_ARGUMENT);
    ensure (info . group < DNA_MAX_GROUP, DNA_BAD_ARGUMENT);

    ensure (p_fd != NULL, DNA_BAD_ARGUMENT);
    ensure (p_file != NULL, DNA_BAD_ARGUMENT);

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& file_pool . lock);

    /*
     * Find a free file descriptor.
     */

    file_array = file_pool . file[info . group];

    for (fd = 0; file_array[fd] != NULL && fd < DNA_MAX_FILE; fd += 1);
    check (error, fd != DNA_MAX_FILE, DNA_MAX_OPENED_FILES);

    /*
     * Allocate the new file.
     */
#if 1
    {
    struct _file* kalloc_addr;   
    kalloc_addr = kernel_malloc (sizeof (struct _file)+0x1F, true);
    file = (void*)(((uintptr_t)kalloc_addr + 0x1F) & ~(uintptr_t)0x1F);
    file->kalloc_addr = kalloc_addr;
     }
    DCACHE_FLUSH(&file->kalloc_addr,sizeof(file_t*));
#else
    file = kernel_malloc (sizeof (struct _file), true);
#endif   
    dna_printf("file %p kalloc %p\n", file, file->kalloc_addr);
    check (error, file != NULL, DNA_OUT_OF_MEM);

    file -> vnode = vnode;
    DCACHE_FLUSH(&file->vnode,sizeof(vnode_t));
    file -> mode = mode;
    DCACHE_FLUSH(&file->mode, sizeof(uint32_t));
    file -> data = data;
    DCACHE_FLUSH(&file->data,sizeof(void*));
    file -> usage_counter = 1;
    DCACHE_FLUSH(&file->usage_counter,sizeof(uint32_t));
    file_pool . file[info . group][fd] = file;
    DCACHE_FLUSH(&file_pool.file[info.group][fd],sizeof(file_t)); // file_t is a pointer type!

    lock_release (& file_pool . lock);
    cpu_trap_restore (it_status);

    /*
     * Return the new file and the new FD.
     */

    dna_log(VERBOSE_LEVEL, "Created FD (%d, 0x%x).", fd, file);

    *p_fd = fd;
    *p_file = file;

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

