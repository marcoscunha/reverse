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

status_t devfs_entry_remove_by_name (devfs_inode_t node, char * name)
{
  devfs_entry_t entry;
  status_t status;

  watch (status_t)
  {
    ensure (node != NULL, DNA_BAD_ARGUMENT);
    ensure (name != NULL, DNA_BAD_ARGUMENT);

    entry = queue_lookup (& node -> entry_list,
        devfs_entry_name_inspector, name);
    ensure (entry != NULL, DNA_NO_ENTRY);

    status = queue_extract (& node -> entry_list, entry);
    ensure (status == DNA_OK, DNA_NO_ENTRY);
 
    kernel_free (entry);
    return DNA_OK;
  }
}
