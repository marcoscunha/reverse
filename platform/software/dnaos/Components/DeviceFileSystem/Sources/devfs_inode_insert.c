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

status_t devfs_inode_insert (devfs_t fs, devfs_inode_t inode,
    char * path, device_cmd_t * commands)
{
  devfs_entry_t entry = NULL;
  devfs_inode_t next_inode = NULL;
  char token[DNA_FILENAME_LENGTH];
  status_t status = DNA_OK;

  watch (status_t)
  {
    ensure (path != NULL, DNA_BAD_ARGUMENT);

    path_get_next_entry (& path, token);
    entry = queue_lookup (& inode -> entry_list,
        devfs_entry_name_inspector, token);

    if (entry == NULL)
    {
      if (dna_strlen (path) != 0)
      {
        devfs_inode_create (fs, inode, DNA_DEVFS_DIRECTORY,
            token, fs -> inode_index ++, NULL, & next_inode);

        devfs_entry_add (next_inode, ".", next_inode -> id);
        devfs_entry_add (next_inode, "..", inode -> id);

        devfs_inode_insert (fs, next_inode, path, commands);
      }
      else
      {
        devfs_inode_create (fs, inode, DNA_DEVFS_FILE, token,
            fs -> inode_index ++, commands, & next_inode);
      }

      devfs_entry_add (inode, token, next_inode -> id);
      dna_log(VERBOSE_LEVEL, "inode %s created", token);
    }
    else
    {
      ensure (dna_strlen (path) != 0, DNA_INODE_EXISTS);

      /*
       * Load the corresponding inode.
       */

      next_inode = queue_lookup (& fs -> inode_list,
          devfs_inode_id_inspector, entry -> id);
      ensure (next_inode != NULL, DNA_NO_VNODE);
      ensure (next_inode -> class == DNA_DEVFS_DIRECTORY, DNA_BAD_INODE_TYPE);

      /*
       * Parse the rest of the path.
       */

      status = devfs_inode_insert (fs, next_inode, path, commands);
    }

    return status;
  }
}

