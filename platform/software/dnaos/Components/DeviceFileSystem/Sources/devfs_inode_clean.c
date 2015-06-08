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

status_t devfs_inode_clean (devfs_t fs, devfs_inode_t inode)
{
  devfs_inode_t next_inode = NULL;
  devfs_entry_t entry = NULL;
  status_t status = DNA_OK;

  watch (status_t)
  {
    dna_log(VERBOSE_LEVEL, "Clean inode [%s] forward.", inode -> name);

    entry = queue_lookup (& inode -> entry_list, devfs_entry_unused_inspector);
    while (entry != NULL)
    {
      dna_log(VERBOSE_LEVEL, "Unused entry [%s].", entry -> name);

      next_inode = queue_lookup (& fs -> inode_list,
          devfs_inode_id_inspector, entry -> id);
      ensure (next_inode != NULL, DNA_NO_VNODE);

      status = devfs_inode_destroy (fs, next_inode);
      ensure (status == DNA_OK, status);

      status = devfs_entry_remove_by_name (inode, next_inode -> name);
      ensure (status == DNA_OK, status);

      entry = queue_lookup (& inode -> entry_list,
          devfs_entry_unused_inspector);
    }

    dna_log(VERBOSE_LEVEL, "Clean inode [%s] backward.", inode -> name);

    if (inode -> entry_list . status == 0 && inode -> parent != NULL)
    {
      entry = queue_lookup (& inode -> parent -> entry_list,
          devfs_entry_name_inspector, inode -> name);
      ensure (entry != NULL, DNA_NO_ENTRY);

      status = devfs_entry_remove_by_name (inode -> parent, inode -> name);
      ensure (status == DNA_OK, status);

      status = devfs_inode_destroy (fs, inode);
      ensure (status == DNA_OK, status);

      status = devfs_inode_clean (fs, inode -> parent);
      ensure (status == DNA_OK, status);
    }

    return DNA_OK;
  }
}

