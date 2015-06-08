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
#include <Private/Media.h>
#include <Private/FATlib.h>
#include <DnaTools/DnaTools.h>
#include <MemoryManager/MemoryManager.h>

/****f* FATFileSystem/fatfs_mount
 * SUMMARY
 * Mount a FAT volume.
 *
 * SYNOPSIS
 */

status_t fatfs_mount (int32_t vid, const char * dev_path, uint32_t flags, 
	void * params, void ** data, int64_t * vnid)
	
/*  
 * ARGUMENTS
 * * vid : the volume id
 * * dev_path : path to the device
 * * flags : unused
 * * params : unused
 * * data : the namespace of the mounted volume (fatfs_t)
 * * vnid : the inode of root inode
 *
 * FUNCTION
 * Mount a FAT volume.
 * Create the namespace and create the root vnode of the volume.
 * This function is called by vfs_mount().
 *
 * RESULT
 * * DNA_OK if the operation succeed
 * * DNA_ERROR if an error occured
 * * DNA_BAD_ARGUMENT if an argument
 * * DNA_OUT_OF_MEM the memory allocation failed
 *
 * SOURCE
 */

{
	fatfs_t fatfs = NULL;
	fatfs_inode_t root_inode = NULL;
	
	status_t status;

 dna_log(VERBOSE_LEVEL, "[start] FATFS mount (dev_path = %s)", dev_path);

	watch(status_t)
	{
		ensure (dev_path != NULL, DNA_BAD_ARGUMENT);

		/*
		 * create the fatfs structure
		 */
		fatfs = kernel_malloc (sizeof (struct fatfs), true);
		ensure (fatfs != NULL, DNA_OUT_OF_MEM);
	
		/*
		 * open and get the file descriptor of the device
		 */
		status = (status_t)media_open(dev_path, &(fatfs->fs_fd));
		check(source_error, status == 1, DNA_ERROR);
	
		/*
		 * attach opening/writing functions
		 */
		fatfs -> disk_io.read_sector = media_read;
		fatfs -> disk_io.write_sector = media_write;

		/*
		 * initialize the fatfs structure
		 */
		status = (status_t)fatfs_init(fatfs);
		check(source_error, status == FAT_INIT_OK, DNA_ERROR); 

		/*
		 * set volume id 
		 */
		fatfs -> vid = vid;
		
		/*
		 * set cluster root vnode id
		 */
		fatfs -> root_vnid = (((uint64_t)fatfs_get_root_cluster(fatfs)) << 32) + 0xFFFFFFFF;
			
		/*
		 * return values
		 */
			
		*data = fatfs;
		*vnid = fatfs -> root_vnid;
		
		/* 
		 * get cluster root vnode
		 */
		status = fatfs_read_vnode(fatfs, fatfs -> root_vnid, (void **)& root_inode);
		check(source_error, status == 0, DNA_ERROR);
	
 dna_log(VERBOSE_LEVEL, "[end] FATFS mount");
		
		/*
		 * create the root vnode_create
		 */

		return vnode_create (fatfs -> root_vnid, fatfs -> vid, (void *) root_inode);
	}
	
	rescue (source_error)
	{
		kernel_free (fatfs);
		leave;
	}
}

/*
 ****/

