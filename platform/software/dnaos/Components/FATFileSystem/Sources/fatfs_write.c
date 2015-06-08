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
#include <DnaTools/DnaTools.h>

/****f* FATFileSystem/fatfs_write
 * SUMMARY
 * Write data corresponding to an inode on a FAT volume.
 *
 * SYNOPSIS
 */

status_t fatfs_write (void * ns, void * node, void * file, 
  void * data, int64_t offset, int32_t * p_count)

/*  
 * ARGUMENTS
 * * ns : the namespace (fatfs_t)
 * * node : the inode (fatfs_inode_t)
 * * file : the file entry (fatfs_entry_t)
 * * data : the data to write
 * * offset : the first byte of the data from the begining of the file
 * * p_count : the size of the data to write / the size of the data written
 * 	
 * FUNCTION
 * Write data corresponding to an inode on a FAT volume.
 * This function is called by fvs_write().
 * Not implemented.
 *
 * RESULT
 * * DNA_OK
 *
 * SOURCE
 */

{
  return DNA_OK;
}

/*
 ****/



