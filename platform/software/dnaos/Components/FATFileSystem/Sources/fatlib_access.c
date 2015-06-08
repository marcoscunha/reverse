/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
					        FAT16/32 File IO Library
								    V2.6
 	  							 Rob Riglar
						    Copyright 2003 - 2010

   					  Email: rob@robriglar.com

								License: GPL
   If you would like a version with a more permissive license for use in
   closed source commercial applications please contact me for details.
-----------------------------------------------------------------------------

 This file is part of FAT File IO Library.

 FAT File IO Library is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 FAT File IO Library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with FAT File IO Library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/

#include <Private/fatlib_access.h>
#include <Private/fatlib_table.h>
#include <Private/fatlib_misc.h>

/*-----------------------------------------------------------------------------
 fatfs_init: Load FAT Parameters
-----------------------------------------------------------------------------*/
int16_t fatfs_init(struct fatfs *fs)
{
	uint8_t	 num_of_fats;
	uint16_t reserved_sectors;
	uint32_t FATSz;
	uint32_t root_dir_sectors;
	uint32_t total_sectors;
	uint32_t data_sectors;
	uint32_t count_of_clusters;
	uint32_t partition_size = 0;
	uint8_t  valid_partition = 0;

	fs->currentsector.address = FAT32_INVALID_CLUSTER;
	fs->currentsector.dirty = 0;

	fs->next_free_cluster = 0; /* Invalid */

	fatfs_fat_init(fs);

	/* Make sure we have read and write functions */
	if (!fs->disk_io.read_sector || !fs->disk_io.write_sector)
		return FAT_INIT_MEDIA_ACCESS_ERROR;

	/* MBR: Sector 0 on the disk */
	/* NOTE: Some removeable media does not have this. */

	/* Load MBR (LBA 0) into the 512 byte buffer */
	if (!fs->disk_io.read_sector(0, fs->currentsector.sector, fs->fs_fd))
		return FAT_INIT_MEDIA_ACCESS_ERROR;
		
	/* Make Sure 0x55 and 0xAA are at end of sector */
	/* (this should be the case regardless of the MBR or boot sector) */
	if (fs->currentsector.sector[SIGNATURE_POSITION] != 0x55 || fs->currentsector.sector[SIGNATURE_POSITION+1] != 0xAA)
		return FAT_INIT_INVALID_SIGNATURE;
		
	/* Now check again using the access function to prove endian conversion function */
	if (GET_16BIT_WORD(fs->currentsector.sector, SIGNATURE_POSITION) != SIGNATURE_VALUE)
		return FAT_INIT_ENDIAN_ERROR;

	/* Check the partition type code */
	switch(fs->currentsector.sector[PARTITION1_TYPECODE_LOCATION])
	{
		case 0x0B:
		case 0x06:
		case 0x0C:
		case 0x0E:
		case 0x0F:
		case 0x05:
			valid_partition = 1;
		break;
		case 0x00:
			valid_partition = 0;
			break;
		default:
			if (fs->currentsector.sector[PARTITION1_TYPECODE_LOCATION] <= 0x06)
				valid_partition = 1;
		break;
	}
	
	
	if (valid_partition)
	{
		/* Read LBA Begin for the file system */
		fs->lba_begin = GET_32BIT_WORD(fs->currentsector.sector, PARTITION1_LBA_BEGIN_LOCATION);
		partition_size = GET_32BIT_WORD(fs->currentsector.sector, PARTITION1_SIZE_LOCATION);
	}
	/* Else possibly MBR less disk */
	else
		fs->lba_begin = 0;

	/* Load Volume 1 table into sector buffer */
	/* (We may already have this in the buffer if MBR less drive!) */
	
	if (!fs->disk_io.read_sector(fs->lba_begin, fs->currentsector.sector, fs->fs_fd))
		return FAT_INIT_MEDIA_ACCESS_ERROR;

	/* Make sure there are 512 bytes per cluster */
	if (GET_16BIT_WORD(fs->currentsector.sector, 0x0B) != FAT_SECTOR_SIZE)
		return FAT_INIT_INVALID_SECTOR_SIZE;

	/* Load Parameters of FAT partition	*/
	fs->sectors_per_cluster = fs->currentsector.sector[BPB_SECPERCLUS];
	reserved_sectors = GET_16BIT_WORD(fs->currentsector.sector, BPB_RSVDSECCNT);
	num_of_fats = fs->currentsector.sector[BPB_NUMFATS];
	fs->root_entry_count = GET_16BIT_WORD(fs->currentsector.sector, BPB_ROOTENTCNT);

	if(GET_16BIT_WORD(fs->currentsector.sector, BPB_FATSZ16) != 0)
		fs->fat_sectors = GET_16BIT_WORD(fs->currentsector.sector, BPB_FATSZ16);
	else
		fs->fat_sectors = GET_32BIT_WORD(fs->currentsector.sector, BPB_FAT32_FATSZ32);

	/* For FAT32 (which this may be) */
	fs->rootdir_first_cluster = GET_32BIT_WORD(fs->currentsector.sector, BPB_FAT32_ROOTCLUS);
	fs->fs_info_sector = GET_16BIT_WORD(fs->currentsector.sector, BPB_FAT32_FSINFO);

	/* For FAT16 (which this may be), rootdir_first_cluster is actuall rootdir_first_sector */
	fs->rootdir_first_sector = reserved_sectors + (num_of_fats * fs->fat_sectors);
	fs->rootdir_sectors = ((fs->root_entry_count * 32) + (FAT_SECTOR_SIZE - 1)) / FAT_SECTOR_SIZE;

	/* First FAT LBA address */
	fs->fat_begin_lba = fs->lba_begin + reserved_sectors;

	/* The address of the first data cluster on this volume */
	fs->cluster_begin_lba = fs->fat_begin_lba + (num_of_fats * fs->fat_sectors);

	if (GET_16BIT_WORD(fs->currentsector.sector, 0x1FE) != 0xAA55) /* This signature should be AA55 */
		return FAT_INIT_INVALID_SIGNATURE;

	/* Calculate the root dir sectors */
	root_dir_sectors = ((GET_16BIT_WORD(fs->currentsector.sector, BPB_ROOTENTCNT) * 32) + (GET_16BIT_WORD(fs->currentsector.sector, BPB_BYTSPERSEC) - 1)) / GET_16BIT_WORD(fs->currentsector.sector, BPB_BYTSPERSEC);

	if(GET_16BIT_WORD(fs->currentsector.sector, BPB_FATSZ16) != 0)
		FATSz = GET_16BIT_WORD(fs->currentsector.sector, BPB_FATSZ16);
	else
		FATSz = GET_32BIT_WORD(fs->currentsector.sector, BPB_FAT32_FATSZ32);

	if(GET_16BIT_WORD(fs->currentsector.sector, BPB_TOTSEC16) != 0)
		total_sectors = GET_16BIT_WORD(fs->currentsector.sector, BPB_TOTSEC16);
	else
		total_sectors = GET_32BIT_WORD(fs->currentsector.sector, BPB_TOTSEC32);

	data_sectors = total_sectors - (GET_16BIT_WORD(fs->currentsector.sector, BPB_RSVDSECCNT) + (fs->currentsector.sector[BPB_NUMFATS] * FATSz) + root_dir_sectors);

	/* Find out which version of FAT this is... */
	if (fs->sectors_per_cluster != 0)
	{
		count_of_clusters = data_sectors / fs->sectors_per_cluster;

		if(count_of_clusters < 4085)
			/* Volume is FAT12 */
			return FAT_INIT_WRONG_FILESYS_TYPE;
		else if(count_of_clusters < 65525)
		{
			/* Clear this FAT32 specific param */
			fs->rootdir_first_cluster = 0;

			/* Volume is FAT16 */
			fs->fat_type = FAT_TYPE_16;
			return FAT_INIT_OK;
		}
		else
		{
			/* Volume is FAT32 */
			fs->fat_type = FAT_TYPE_32;
			return FAT_INIT_OK;
		}
	}
	else
		return FAT_INIT_WRONG_FILESYS_TYPE;
}

/*-----------------------------------------------------------------------------
 fatfs_lba_of_cluster: This function converts a cluster number into a sector
 LBA number.
-----------------------------------------------------------------------------*/
uint32_t fatfs_lba_of_cluster(struct fatfs *fs, uint32_t Cluster_Number)
{
	if (fs->fat_type == FAT_TYPE_16)
		return (fs->cluster_begin_lba + (fs->root_entry_count * 32 / FAT_SECTOR_SIZE) + ((Cluster_Number-2) * fs->sectors_per_cluster));
	else
		return ((fs->cluster_begin_lba + ((Cluster_Number-2)*fs->sectors_per_cluster)));
}

/*-----------------------------------------------------------------------------
 fatfs_sector_reader: From the provided startcluster and sector offset
 Returns True if success, returns False if not (including if read out of range)
-----------------------------------------------------------------------------*/
int16_t fatfs_sector_reader(struct fatfs *fs, uint32_t Startcluster, uint32_t offset, unsigned char *target)
{
	uint32_t SectortoRead = 0;
	uint32_t ClustertoRead = 0;
	uint32_t ClusterChain = 0;
	uint32_t i;
	uint32_t lba;

	/* FAT16 Root directory */
	if (fs->fat_type == FAT_TYPE_16 && Startcluster == 0)
	{
		if (offset < fs->rootdir_sectors)
			lba = fs->lba_begin + fs->rootdir_first_sector + offset;
		else
			return 0;
	}
	/* FAT16/32 Other */
	else
	{
		/* Set start of cluster chain to initial value */
		ClusterChain = Startcluster;

		/* Find parameters */
		ClustertoRead = offset / fs->sectors_per_cluster;
		SectortoRead = offset - (ClustertoRead*fs->sectors_per_cluster);

		/* Follow chain to find cluster to read */
		for (i=0; i<ClustertoRead; i++)
			ClusterChain = fatfs_find_next_cluster(fs, ClusterChain);

		/* If end of cluster chain then return false */
		if (ClusterChain == FAT32_LAST_CLUSTER)
			return 0;

		/* Calculate sector address */
		lba = fatfs_lba_of_cluster(fs, ClusterChain)+SectortoRead;
	}

	/* User provided target array */
	if (target)
		return fs->disk_io.read_sector(lba, target, fs->fs_fd);
	/* Else read sector if not already loaded */
	else if (lba != fs->currentsector.address)
	{
		fs->currentsector.address = lba;
		return fs->disk_io.read_sector(fs->currentsector.address, fs->currentsector.sector, fs->fs_fd);
	}
	else
		return 1;
}

/*-----------------------------------------------------------------------------
 fatfs_get_root_cluster: Get the root dir cluster
----------------------------------------------------------------------------- */
uint32_t fatfs_get_root_cluster(struct fatfs *fs)
{
	/* NOTE: On FAT16 this will be 0 which has a special meaning... */
	return fs->rootdir_first_cluster;
}

