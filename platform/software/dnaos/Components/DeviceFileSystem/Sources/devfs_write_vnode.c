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

status_t devfs_write_vnode (void * ns, void * node)
{
  status_t status = DNA_OK;
  devfs_t devfs = ns;
  devfs_inode_t inode = node;

  watch (status_t)
  {
    dna_log(VERBOSE_LEVEL, "Write inode [%s].", inode -> name);

    status = devfs_inode_clean (devfs, inode);
    ensure (status == DNA_OK, status);

    return DNA_OK;
  }
}
