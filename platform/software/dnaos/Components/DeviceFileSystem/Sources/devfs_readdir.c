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

status_t devfs_readdir (void * ns, void * node, void * data,
    void * entry_array, int64_t * offset, int32_t * p_count)
{
  devfs_t devfs = ns;
  devfs_inode_t inode = node;
  devfs_entry_t entry = NULL;
  status_t status = DNA_OK;
  int64_t pos = 0;
  directory_entry_t * p_entry = entry_array;

  watch (status_t)
  {
    ensure (ns != NULL && node != NULL
        && entry_array != NULL && p_count != NULL, DNA_ERROR);
    ensure (inode -> class == DNA_DEVFS_DIRECTORY, DNA_BAD_INODE_TYPE);

    if (inode -> entry_list . status == 0)
    {
      *p_count = 0;
    }
    else
    {
      pos = * offset;
      entry = queue_lookup (& inode -> entry_list,
          devfs_entry_index_inspector, & pos);

      if (entry == NULL)
      {
        *p_count = 0;
      }
      else
      {
        p_entry -> vnid = entry -> id;
        p_entry -> vid = devfs -> vid;
        dna_strcpy (p_entry -> d_name, entry -> name);

        p_entry -> d_reclen = sizeof (struct _directory_entry)
          + dna_strlen (entry -> name);

        *p_count = p_entry -> d_reclen;
        *offset += 1;
      }
    }

    return status;
  }
}

