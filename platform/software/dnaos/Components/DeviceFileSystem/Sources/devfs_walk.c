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

#include <stdbool.h>
#include <Private/DeviceFileSystem.h>
#include <DnaTools/DnaTools.h>
#include <MemoryManager/MemoryManager.h>

status_t devfs_walk (void * ns, void * base, char * path,
    char ** new_path, int64_t * p_vnid)
{
  devfs_t devfs = ns;
  devfs_inode_t inode = base;
  devfs_entry_t entry = NULL;
  device_cmd_t * commands = NULL;
  char ** devices = NULL, * a_path = NULL;
  int32_t driver_index = 0;

  watch (status_t)
  {
    ensure (ns != NULL && base != NULL, DNA_ERROR);
    dna_log(VERBOSE_LEVEL, "Analyzing \"%s\" on vnode %lld", path, inode -> id);

    if (inode -> id == devfs -> root_vnid)
    {
      ensure (dna_strcmp (path, "..") != 0, DNA_ALREADY_AT_ROOT);

     /*
      * Parse the publish_devices.
      */

      a_path = kernel_malloc (DNA_PATH_LENGTH, false);
      ensure (a_path != NULL, DNA_OUT_OF_MEM);

      for (driver_index = 0; driver_index < OS_N_DRIVERS; driver_index ++)
      {
        devices = (char **) OS_DRIVERS_LIST[driver_index] -> publish_devices ();

        for (int32_t j = 0; devices[j] != NULL; j ++)
        {
          if (dna_memcmp (path, devices[j], dna_strlen(path)) == 0)
          {
            dna_strcpy (a_path, devices[j]);
            commands = OS_DRIVERS_LIST[driver_index] -> find_device (a_path);
            devfs_inode_insert (devfs, inode, a_path, commands);
          }
        }
      }

      kernel_free (a_path);
    }

    /*
     * Check whether the dummy inode is empty or not.
     * If everything is fine, get the path vnid.
     */

    entry = queue_lookup (& inode -> entry_list,
        devfs_entry_name_inspector, path);
    ensure (entry != NULL, DNA_NO_ENTRY);

    *p_vnid = entry -> id;
    return DNA_OK;
  }
}

