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
#include <Private/FATlib.h>

/****f* FATFileSystem/fatfs_walk
 * SUMMARY
 * Return the id corresponding to a path from a base inode on a FAT volume.
 *
 * SYNOPSIS
 */

status_t fatfs_walk (void * ns, void * base, char * restrict path,
    char ** new_path, int64_t * p_vnid)
    
/*  
 * ARGUMENTS
 * * ns : the namespace (fatfs_t)
 * * base : the base inode (fatfs_inode_t)
 * * path : the path
 * * new_path : unused
 * * p_vnid : the id corresponding to the path
 * 	
 * FUNCTION
 * Return the id corresponding to a path from a base inode on a FAT volume.
 * This function is called by vfs_walk().
 *
 * RESULT
 * * DNA_OK if the operation succeed
 * * DNA_BAD_ARGUMENT if an argument is missing
 * * DNA_ALREADY_AT_ROOT if the base inode is the root inode and the path is ..
 * * DNA_NO_ENTRY if no match found
 * * DNA_BAD_INODE_TYPE if the base inode is not a directory inode
 *
 * SOURCE
 */
    
{
//  fatfs_t fatfs = ns;
  fatfs_inode_t base_inode = base;
  int64_t vnid = -1;
  uint16_t recordoffset = 0;
  uint8_t item = 0;
  uint16_t sector = 0;
  fatfs_entry_t directoryEntry;
  char filename[261] = {0};

  dna_log(VERBOSE_LEVEL, "[start] FATFS walk (path = %s)", path);

  watch (status_t)
  {
    ensure (ns != NULL && base != NULL, DNA_BAD_ARGUMENT);
    
	/*
	 * check if the inode is an inode directory
	 */
    ensure (base_inode -> cluster_chain_directory != NULL, 
    	DNA_BAD_INODE_TYPE);
    	
    /*
     * the path is the current directory ?
     */
    if(dna_strcmp (path, ".") == 0)
    {
    	vnid = base_inode->id;
    }
    /*
     * no, so search the entry
     */    
    else 
	{
		/*
		 * walk the inode cluster chain
		 */
		for(sector = 0; sector < base_inode -> nb_sector; sector++)
		{
			/*
			 * Analyse Sector
			 */
			for (item = 0; item < 16; item++)
			{
				/*
				 * Create the multiplier for sector access
				 */
				recordoffset = (32*item);

				/*
				 * Overlay directory entry over buffer
				 */
				directoryEntry = (fatfs_entry_t)(base_inode -> cluster_chain_directory 
					+ sector*FAT_SECTOR_SIZE + recordoffset);
					
				if(fatfs_get_fn_entry(directoryEntry, filename) == 0)
				{
					/*
					 * Compare names to see if they match
					 */
					if (fatfs_compare_names(filename, path) == 0)
					{
						vnid = ((uint64_t)base_inode -> cc_dirid << 32) + 
							sector*FAT_SECTOR_SIZE + recordoffset;
						break;
					}
					
					fatfs_reset_lfn(filename);
				}
			}
		}
    }

	/*
	 * entry not found, check if the path is ".."
	 */
	ensure (vnid != -1 || dna_strcmp (path, "..") != 0,
		 DNA_ALREADY_AT_ROOT);
    ensure (vnid != -1, DNA_NO_ENTRY);
    
    /*
     * return the vnid
     */
    
    *p_vnid = vnid;

    dna_log(VERBOSE_LEVEL, "[end] FATFS walk (vnid = 0x%llx)", vnid);
    
    return DNA_OK;
  }
}

/*
 ****/

