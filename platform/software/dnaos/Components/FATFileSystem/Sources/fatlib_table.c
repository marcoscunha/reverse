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
/*----------------------------------------------------------------------------- */

#include <Private/fatlib_defs.h>
#include <Private/fatlib_access.h>
#include <Private/fatlib_table.h>

#ifndef FAT_BUFFERED_SECTORS
	#define FAT_BUFFERED_SECTORS 1
#endif

#if FAT_BUFFERED_SECTORS < 1
	#error "FAT_BUFFERED_SECTORS must be at least 1"
#endif

/*----------------------------------------------------------------------------- */
/*							FAT Sector Buffer */
/*----------------------------------------------------------------------------- */
#define FAT32_GET_32BIT_WORD(pbuf, location)	( GET_32BIT_WORD(pbuf->sector, location) )
#define FAT32_SET_32BIT_WORD(pbuf, location, value)	{ SET_32BIT_WORD(pbuf->sector, location, value); pbuf->dirty = 1; }
#define FAT16_GET_16BIT_WORD(pbuf, location)	( GET_16BIT_WORD(pbuf->sector, location) )
#define FAT16_SET_16BIT_WORD(pbuf, location, value)	{ SET_16BIT_WORD(pbuf->sector, location, value); pbuf->dirty = 1; }

/*----------------------------------------------------------------------------- */
/* fatfs_fat_init: */
/*----------------------------------------------------------------------------- */
void fatfs_fat_init(struct fatfs *fs)
{
	int i;

	/* FAT buffer chain head */
	fs->fat_buffer_head = NULL;

	for (i=0;i<FAT_BUFFERED_SECTORS;i++)
	{
		/* Initialise buffers to invalid */
		fs->fat_buffers[i].address = FAT32_INVALID_CLUSTER;
		fs->fat_buffers[i].dirty = 0;
		dna_memset(fs->fat_buffers[i].sector, 0x00, sizeof(fs->fat_buffers[i].sector));

		/* Add to head of queue */
		fs->fat_buffers[i].next = fs->fat_buffer_head;
		fs->fat_buffer_head = &fs->fat_buffers[i];
	}
}
/*----------------------------------------------------------------------------- */
/* fatfs_fat_read_sector: Read a FAT sector */
/*----------------------------------------------------------------------------- */
static struct sector_buffer	* fatfs_fat_read_sector(struct fatfs *fs, uint32_t sector)
{
	struct sector_buffer *last = NULL;
	struct sector_buffer *pcur = fs->fat_buffer_head;

	/* Itterate through sector buffer list */
	while (pcur)
	{
		/* Sector already in sector list */
		if (pcur->address == sector)
			break;

		/* End of list? */
		if (pcur->next == NULL)
		{
			/* Remove buffer from list */
			if (last)
				last->next = NULL;
			/* We the first and last buffer in the chain? */
			else
				fs->fat_buffer_head = NULL;
		}

		last = pcur;
		pcur = pcur->next;
	}

	/* We found the sector already in FAT buffer chain */
	if (pcur)
		return pcur;

	/* Else, we removed the last item from the list */
	pcur = last;

	/* Add to start of sector buffer list (now newest sector) */
	pcur->next = fs->fat_buffer_head;
	fs->fat_buffer_head = pcur;

	/* Writeback sector if changed */
	if (pcur->dirty)
	{
		if (!fs->disk_io.write_sector(pcur->address, pcur->sector, fs->fs_fd))
			return NULL;

		/* Now no longer 'dirty' */
		pcur->dirty = 0;
	}

	/* Address is now new sector */
	pcur->address = sector;

	/* Read next sector */
	if (!fs->disk_io.read_sector(pcur->address, pcur->sector, fs->fs_fd))
	{
		/* Read failed, invalidate buffer address */
		pcur->address = FAT32_INVALID_CLUSTER;
		return NULL;
	}

	return pcur;
}

/*----------------------------------------------------------------------------- */
/*						General FAT Table Operations */
/*----------------------------------------------------------------------------- */

/*----------------------------------------------------------------------------- */
/* fatfs_find_next_cluster: Return cluster number of next cluster in chain by */
/* reading FAT table and traversing it. Return 0xffffffff for end of chain. */
/*----------------------------------------------------------------------------- */
uint32_t fatfs_find_next_cluster(struct fatfs *fs, uint32_t current_cluster)
{
	uint32_t fat_sector_offset, position;
	uint32_t nextcluster;
	struct sector_buffer *pbuf;

	/* Why is '..' labelled with cluster 0 when it should be 2 ?? */
	if (current_cluster == 0)
		current_cluster = 2;

	/* Find which sector of FAT table to read */
	if (fs->fat_type == FAT_TYPE_16)
		fat_sector_offset = current_cluster / 256;
	else
		fat_sector_offset = current_cluster / 128;

	/* Read FAT sector into buffer */
	pbuf = fatfs_fat_read_sector(fs, fs->fat_begin_lba+fat_sector_offset);
	if (!pbuf)
		return (FAT32_LAST_CLUSTER);

	if (fs->fat_type == FAT_TYPE_16)
	{
		/* Find 32 bit entry of current sector relating to cluster number */
		position = (current_cluster - (fat_sector_offset * 256)) * 2;

		/* Read Next Clusters value from Sector Buffer */
		nextcluster = FAT16_GET_16BIT_WORD(pbuf, (uint16_t)position);

		/* If end of chain found */
		if (nextcluster >= 0xFFF8 && nextcluster <= 0xFFFF)
			return (FAT32_LAST_CLUSTER);
	}
	else
	{
		/* Find 32 bit entry of current sector relating to cluster number */
		position = (current_cluster - (fat_sector_offset * 128)) * 4;

		/* Read Next Clusters value from Sector Buffer */
		nextcluster = FAT32_GET_32BIT_WORD(pbuf, (uint16_t)position);

		/* Mask out MS 4 bits (its 28bit addressing) */
		nextcluster = nextcluster & 0x0FFFFFFF;

		/* If end of chain found */
		if (nextcluster >= 0x0FFFFFF8 && nextcluster <= 0x0FFFFFFF)
			return (FAT32_LAST_CLUSTER);
	}

	/* Else return next cluster */
	return (nextcluster);
}

/*
 ****/

