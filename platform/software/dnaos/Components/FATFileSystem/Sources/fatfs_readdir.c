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

/****f* FATFileSystem/fatfs_readdir
 * SUMMARY
 * Return a directory entry info from an inode on a FAT volume.
 *
 * SYNOPSIS
 */

status_t fatfs_readdir (void * ns, void * node, void * data,
    void * entry_array, int64_t * offset, int32_t * p_count)
   
/*  
 * ARGUMENTS
 * * ns : the namespace (fatfs_t)
 * * node : the inode of the directory (fatfs_inode_t)
 * * data : unused
 * * entry_array : return data (directory_entry_t)
 * * offset : byte offset of the next directory entry
 * * p_count : size of the returned data
 *
 * FUNCTION
 * Return a directory entry info from an inode on a FAT volume.
 * Return an entry and point to the next one. 
 * This function is called by vfs_readdir().
 *
 * RESULT
 * * DNA_OK if the operation succeed
 * * DNA_BAD_ARGUMENT if an argument is missing
 *
 * SOURCE
 */
    
{
	fatfs_t fatfs = ns;
	fatfs_inode_t inode = node;
	directory_entry_t * p_entry = entry_array;
	uint8_t item = 0;
	uint16_t recordoffset = 0;
	uint16_t sector=0;
	char filename[261] = { 0 };
	uint16_t item_start;
	fatfs_entry_t directoryEntry;
	
 dna_log(VERBOSE_LEVEL, "readdir [start]");
	
	watch (status_t)
	{
		ensure (ns != NULL && node != NULL && entry_array != NULL
			&& p_count != NULL, DNA_BAD_ARGUMENT);
			
		ensure (inode->cluster_chain_directory != NULL, DNA_OK);
   	
		item_start = (*offset)%FAT_SECTOR_SIZE;

		for(sector = (*offset)/FAT_SECTOR_SIZE; sector < inode -> nb_sector; sector++)
		{
			/*
			 * Maximum of 16 directory entries
			 */
			for (item = item_start; item < 16; item++)
			{
				/*
				 * Increase directory offset
				 */
				recordoffset = (32*item);

				/*
				 * Overlay directory entry over buffer
				 */
				directoryEntry = (fatfs_entry_t)(inode -> cluster_chain_directory 
					+ sector*FAT_SECTOR_SIZE + recordoffset);
				
				if(fatfs_get_fn_entry(directoryEntry, filename) == 0)
				{
					/*
					 * return data
					 */
					dna_memcpy (p_entry -> d_name, (char *)filename, FATFS_NAME_LENGTH);
				
				    p_entry -> vnid = ((uint64_t)inode -> cc_dirid << 32) + 
							sector*FAT_SECTOR_SIZE + recordoffset;
				    p_entry -> vid = fatfs -> vid;

				    p_entry -> d_reclen = sizeof (struct _directory_entry)
		    		  + dna_strlen (p_entry -> d_name);
		    		  
					/*
					 * Next starting position
					 */
					*offset = sector*FAT_SECTOR_SIZE + item + 1;
					*p_count = p_entry -> d_reclen;

		 			return DNA_OK;
				}
			}
			
			item_start = 0;
		}
		
		/*
		 * empty directory
		 */
		*p_count = 0;
	}

 dna_log(VERBOSE_LEVEL, "readdir [end]");

    return DNA_OK;
}

/*
 ****/


