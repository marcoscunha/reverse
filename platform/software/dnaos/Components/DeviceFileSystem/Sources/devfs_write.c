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

status_t devfs_write (void * ns, void * node, void * file, void * data,
    int64_t offset, int32_t * p_count)
{
  devfs_inode_t inode = node;

  watch (status_t)
  {
    ensure (inode -> dev_cmd -> write != NULL, DNA_ERROR);
    return inode -> dev_cmd -> write (file, data, offset, p_count);
  }
}

