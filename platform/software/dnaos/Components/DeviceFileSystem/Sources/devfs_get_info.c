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

status_t devfs_get_info (void * ns, void * node,
    void * data, file_info_t * p_info)
{
  devfs_t devfs = ns;
  devfs_inode_t inode = node;

  watch (status_t)
  {
    ensure (devfs != NULL, DNA_BAD_ARGUMENT);
    ensure (inode != NULL, DNA_BAD_ARGUMENT);
    ensure (p_info != NULL, DNA_BAD_ARGUMENT);

    dna_memset (p_info, 0, sizeof (file_info_t));

    p_info -> volume = devfs -> vid;
    p_info -> vnode = inode -> id;

    switch (inode -> class)
    {
      case DNA_DEVFS_FILE :
        {
          int32_t ret;
          device_info_t dev_info;

          va_call (list,
              {
                inode -> dev_cmd -> control (data, DNA_GET_INFO, list, & ret);
              },
              & dev_info);

          switch (dev_info . type)
          {
            case DNA_NETWORK_DEVICE :
            case DNA_CHARACTER_DEVICE :
              p_info -> type = DNA_FILE_SPECIAL_CHARACTER;
              break;

            case DNA_DISK_DEVICE :
            case DNA_WORM_DEVICE :
            case DNA_CD_DEVICE :
              p_info -> type = DNA_FILE_SPECIAL_BLOCK;
              break;

            default :
              p_info -> type = DNA_FILE_REGULAR;
              break;
          }

          break;
        }

      case DNA_DEVFS_DIRECTORY :
        {
          p_info -> type = DNA_FILE_DIRECTORY;
          break;
        }

      case DNA_DEVFS_SYMLINK :
        {
          p_info -> type = DNA_FILE_SYMBOLIC_LINK;
          break;
        }
    }

    return DNA_OK;
  }
}

