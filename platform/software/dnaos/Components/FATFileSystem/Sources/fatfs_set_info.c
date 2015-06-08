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

/****f* FATFileSystem/fatfs_set_info
 * SUMMARY
 * Update metadatas of an inode from a FAT volume.
 *
 * SYNOPSIS
 */

status_t fatfs_set_info (void * ns, void * node, void * data,
    file_info_t info, int32_t mask)

/*  
 * ARGUMENTS
 * * ns : the namespace (fatfs_t)
 * * node : the current inode (fatfs_inode_t)
 * * file : the entry of the inode (fatfs_entry_t)
 * * data :
 * * info : the new info
 * * mask : 
 * 	
 * FUNCTION
 * Update metadatas of an inode from a FAT volume.
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


