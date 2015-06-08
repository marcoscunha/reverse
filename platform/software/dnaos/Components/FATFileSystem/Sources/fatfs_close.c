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

#include <Private/FATFileSystem.h>
#include <Private/FATlib.h>
#include <DnaTools/DnaTools.h>

/****f* FATFileSystem/fatfs_close
 * SUMMARY
 * Close a file saved on a FAT volume.
 *
 * SYNOPSIS
 */

status_t fatfs_close (void * ns, void * node, void * data)

/*
 * ARGUMENTS
 * * ns : the namespace (fatfs_t)
 * * node : the inode corresponding to the file (fatfs_inode_t)
 * * data : the entry of the inode (fatfs_entry_t)
 *
 * FUNCTION
 * Close a file saved on FAT volume. Funtion called by vfs_close(). 
 * This funtion do nothing.
 *
 * RESULT
 * * DNA_OK
 *
 * SOURCE
 */

{
  dna_log(VERBOSE_LEVEL, "FATFS close");

  return DNA_OK;
}

/*
 ****/

