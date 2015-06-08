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

status_t rootfs_get_info (void * ns, void * node,
    void * data, file_info_t * p_info)
{
  rootfs_t rootfs = ns;
  rootfs_inode_t inode = node;

  watch (status_t)
  {
    ensure (rootfs != NULL, DNA_BAD_ARGUMENT);
    ensure (inode != NULL, DNA_BAD_ARGUMENT);
    ensure (p_info != NULL, DNA_BAD_ARGUMENT);

    dna_memset (p_info, 0, sizeof (file_info_t));

    p_info -> volume = rootfs -> vid;
    p_info -> vnode = inode -> id;

    switch (inode -> class)
    {
      case DNA_ROOTFS_FILE :
        {
          p_info -> type = DNA_FILE_REGULAR;
          break;
        }

      case DNA_ROOTFS_DIRECTORY :
        {
          p_info -> type = DNA_FILE_DIRECTORY;
          break;
        }

      case DNA_ROOTFS_SYMLINK :
        {
          p_info -> type = DNA_FILE_SYMBOLIC_LINK;
          break;
        }
    }

    return DNA_OK;
  }
}

