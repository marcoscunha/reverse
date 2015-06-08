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

#include <Private/Filesystem.h>
#include <DnaTools/DnaTools.h>

status_t rootfs_read_vnode (void * ns, int64_t vnid, void ** data)
{
  rootfs_t rootfs = ns;
  rootfs_inode_t inode = NULL;

  inode = queue_lookup (& rootfs -> inode_list, rootfs_inode_inspector, vnid);

  dna_log(VERBOSE_LEVEL, "NS = 0x%x, VNID = %lld, DATA = 0x%x",
      ns, vnid, inode);

  *data = inode;
  return DNA_OK;
}

