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

status_t devfs_entry_add (devfs_inode_t inode, char * name, int64_t vnid)
{
  devfs_entry_t entry;

  watch (status_t)
  {
    ensure (inode != NULL, DNA_BAD_ARGUMENT);
    ensure (name != NULL, DNA_BAD_ARGUMENT);

    entry = kernel_malloc (sizeof (struct devfs_entry), true);
    ensure (entry != NULL, DNA_OUT_OF_MEM);

    entry -> id = vnid;
    dna_strcpy (entry -> name, name);
    queue_add (& inode -> entry_list, entry);

    return DNA_OK;
  }
}
