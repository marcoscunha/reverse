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

#include <Private/DeviceFileSystem.h>
#include <DnaTools/DnaTools.h>
#include <MemoryManager/MemoryManager.h>

status_t devfs_inode_destroy (devfs_t fs, devfs_inode_t inode)
{
  status_t status = DNA_OK;
  devfs_entry_t entry;
  devfs_inode_t next_inode = NULL;

  watch (status_t)
  {
    ensure (inode != NULL, DNA_BAD_ARGUMENT);

    dna_log(VERBOSE_LEVEL, "Destroy inode [%s].", inode -> name);

    queue_extract (& fs -> inode_list, inode);
    ensure (status == DNA_OK, status);

    if (inode -> class == DNA_DEVFS_DIRECTORY)
    {
      devfs_entry_remove_by_name (inode, ".");
      devfs_entry_remove_by_name (inode, "..");

      entry = queue_rem (& inode -> entry_list);
      while (entry != NULL)
      {
        next_inode = queue_lookup (& fs -> inode_list,
            devfs_inode_id_inspector, entry -> id);
        ensure (next_inode != NULL, DNA_NO_VNODE);

        status = devfs_inode_destroy (fs, next_inode);
        ensure (status == DNA_OK, status);

        kernel_free (entry);
        entry = queue_rem (& inode -> entry_list);
      }
    }

    kernel_free (inode);
    return DNA_OK;
  }
}
