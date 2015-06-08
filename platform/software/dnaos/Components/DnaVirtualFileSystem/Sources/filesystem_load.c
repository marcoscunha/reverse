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

#include <Private/VirtualFileSystem.h>
#include <DnaTools/DnaTools.h>

/****f* filesystem/filesystem_load
 * SUMMARY
 * Load a filesystem.
 *
 * SYNOPSIS
 */

status_t filesystem_load (char * name, filesystem_t ** fs)

/*
 * SOURCE
 */

{
  filesystem_t * new_fs = NULL;

  watch (status_t)
  {
    dna_log(INFO_LEVEL, "Loading FS \"%s\"", name);

    for (int32_t i = 0; i < OS_N_FILESYSTEMS; i++)
    {
      if (dna_strcmp (OS_FILESYSTEMS_LIST[i] -> name, name) == 0)
      {
        new_fs = OS_FILESYSTEMS_LIST[i];
        break;
      }
    }

    ensure (new_fs != NULL, DNA_ERROR);

    /*
     * Check the consistency of the filesystem.
     */

    for (int32_t i = 0; i < sizeof (filesystem_cmd_t) >> 2; i += 1)
    {
      ensure (((void **)new_fs -> cmd)[i] != NULL, DNA_ERROR);
    }

    *fs = new_fs;
    return DNA_OK;
  }
}

/*
 ****/
