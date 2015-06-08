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
#include <MemoryManager/MemoryManager.h>

/****f* FATFileSystem/fatfs_destroy_vnode
 * SUMMARY
 * Destroy a vnode from a FAT volume.
 *
 * SYNOPSIS
 */

status_t fatfs_destroy_vnode (void * ns, void * node)

/*
 * ARGUMENTS
 * * ns : the namespace (fatfs_t)
 * * node : the inode to free (fatfs_inode_t)
 *
 * FUNCTION
 * Free the memory allocated by fatfs_read_vnode().
 * This function is called by vnode_put().
 *
 * RESULT
 * * DNA_OK if the operation succeed
 * * DNA_BAD_ARGUMENT if an argument is missing
 *
 * SOURCE
 */

{
    fatfs_inode_t inode = node;

 dna_log(VERBOSE_LEVEL, "FATFS write_vnode ");

	watch (status_t)
	{
		ensure (ns != NULL && node != NULL, DNA_BAD_ARGUMENT);
		
		/*
		 * if the inode is a directory, free the cluster chain
		 */

		if (inode -> cluster_chain_directory != NULL)
			kernel_free (inode -> cluster_chain_directory);
		
		/*
		 * free the inode
		 */
		
		kernel_free(inode);
	}
	
	return DNA_OK;
}

/*
 ****/


