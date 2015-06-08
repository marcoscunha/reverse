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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Private/VirtualFileSystem.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

/****f* LifeCyclePrivate/vfs_create
 * SUMMARY
 * Initialize the I/O subsystem
 *
 * SYNOPSIS
 */

status_t vfs_create (void)

/*
 * FUNCTION
 * Initializes the VFS structures.
 *
 * SOURCE
 */

{
  watch (status_t)
  {
    dna_memset (& vnode_manager, 0, sizeof (vnode_manager_t));
    dna_memset (& volume_manager, 0, sizeof (volume_manager_t));

    /*
     * Initialize the file pool.
     */

    dna_memset (& file_pool, 0, sizeof (file_pool_t));

    file_pool . file = kernel_malloc (DNA_MAX_GROUP * sizeof (file_t *), true);
    DCACHE_FLUSH(&file_pool.file, sizeof(file_t));
    ensure (file_pool . file != NULL, DNA_OUT_OF_MEM);

    for (int32_t i = 0; i < DNA_MAX_GROUP; i += 1)
    {
      file_pool . file[i] = kernel_malloc
        (DNA_MAX_FILE * sizeof (file_t), true);
      DCACHE_FLUSH(&file_pool.file[i], sizeof(file_t*)); 
      check (file_no_mem, file_pool . file[i] != NULL, DNA_OUT_OF_MEM);
    }

    return DNA_OK;
  }

  rescue (file_no_mem)
  {
    for (int32_t i = 0; i < DNA_MAX_GROUP; i += 1)
    {
      if (file_pool . file[i] != NULL)
      {
        kernel_free (file_pool . file[i]);
      }
    }

    kernel_free (file_pool . file);
    leave;
  }
}

/*
 ****/

