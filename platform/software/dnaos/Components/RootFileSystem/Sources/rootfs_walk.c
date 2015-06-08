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

status_t rootfs_walk (void * ns, void * base, char * restrict path,
    char ** new_path, int64_t * p_vnid)
{
  rootfs_t rootfs = ns;
  rootfs_inode_t base_inode = base;
  rootfs_entry_t entry = NULL;
  int64_t vnid = -1;

  watch (status_t)
  {
    ensure (ns != NULL && base != NULL, DNA_ERROR);
    ensure (base_inode -> id != rootfs -> root_vnid ||
        dna_strcmp (path, "..") != 0, DNA_ALREADY_AT_ROOT);

    entry = queue_lookup (& base_inode -> entry_list,
        rootfs_entry_name_inspector, path);
    ensure (entry != NULL, DNA_NO_ENTRY);

    vnid = entry -> id;
    *p_vnid = vnid;

    return DNA_OK;
  }
}

