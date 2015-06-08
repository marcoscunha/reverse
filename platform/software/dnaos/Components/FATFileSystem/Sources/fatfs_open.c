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

/****f* FATFileSystem/fatfs_open
 * SUMMARY
 * Open an inode from a FAT volume.
 *
 * SYNOPSIS
 */

status_t fatfs_open (void * ns, void * node, int32_t mode, void ** data)

/*  
 * ARGUMENTS
 * * ns : the namespace (fatfs_t)
 * * node : the current inode (fatfs_inode_t)
 * * mode : unused
 * * data : the inode entry of the current inode (fatfs_entry_t)
 * 	
 * FUNCTION
 * Open an inode from a FAT volume.
 * Get some info about an inode from a FAT volume.
 * This function is called by vfs_open().
 *
 * RESULT
 * * DNA_OK if the operation succeed
 * * DNA_BAD_ARGUMENT if an argument is missing
 *
 * SOURCE
 */

{
    fatfs_inode_t inode = node;

 dna_log(VERBOSE_LEVEL, "FATFS open");

	watch(status_t)
	{
		ensure (ns != NULL && node != NULL, DNA_BAD_ARGUMENT);
		
		/* 
		 * return the inode entry
		 */
		*data = (void *)&(inode -> entry);
	}
	
	return DNA_OK;
}

/*
 ****/

