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

#ifndef FATFS_H
#define FATFS_H

#include <stdarg.h>
#include <DnaTools/DnaTools.h>
#include <VirtualFileSystem/VirtualFileSystem.h>

#include <Private/fatlib_defs.h>

#define FATFS_NAME_LENGTH	DNA_FILENAME_LENGTH

/*----------------------------------------------------------------------------- */
/* Function Pointers */
/*----------------------------------------------------------------------------- */
typedef int (*fn_diskio_read) (const uint32_t sector, unsigned char *buffer, const int32_t fs_fd);
typedef int (*fn_diskio_write)(const uint32_t sector, unsigned char *buffer, const int32_t fs_fd);

/*----------------------------------------------------------------------------- */
/* Structures */
/*----------------------------------------------------------------------------- */
struct disk_if
{
	/* User supplied function pointers for disk IO */
	fn_diskio_read		read_sector;
	fn_diskio_write		write_sector;
};

struct sector_buffer
{
	uint8_t		sector[FAT_SECTOR_SIZE];
	uint32_t	address;
	int16_t		dirty;

	/* Next in chain of sector buffers */
	struct sector_buffer  *next;
};

typedef enum eFatType
{
	FAT_TYPE_16,
	FAT_TYPE_32
} tFatType;

/****f* FATFileSystem/fatfs_entry_t
 * SUMMARY
 * Inode entry defined according to the FAT specifications.
 *
 * SOURCE
 */

typedef struct fatfs_entry
{
  	uint8_t 	Name[11];
	uint8_t 	Attr;
	uint8_t 	NTRes;
	uint8_t 	CrtTimeTenth;
	uint8_t 	CrtTime[2];
	uint8_t 	CrtDate[2];
	uint8_t 	LstAccDate[2];
	uint16_t 	FstClusHI;
	uint8_t 	WrtTime[2];
	uint8_t		WrtDate[2];
	uint16_t 	FstClusLO;
    uint32_t 	FileSize;
}
* fatfs_entry_t;

/*
 ****/


/****f* FATFileSystem/fatfs_inode_t
 * SUMMARY
 * FAT inode type
 *
 * SOURCE
 */

typedef struct fatfs_inode
{
  /* inode id, composed by the cluster number of the parent inode (32 MSB)
    and the byte offset of the inode entry in the parent inode cluster chain directory (32 LSB) */
  int64_t id;
  
  /* cluster number of the cluster chain directory (if the inode is a directory) */
  uint32_t cc_dirid;
  
  /* directory entries of an inode (if the inode is a directory) */
  unsigned char *cluster_chain_directory;
  
  /* size of the cluster chain in sector (if the inode is a directory) */   
  uint32_t nb_sector;
  
  /* entry of the inode */
  struct fatfs_entry entry;
}
* fatfs_inode_t;

/*
 ****/


/****f* FATFileSystem/fatfs_t
 * SUMMARY
 * FAT namespace
 *
 * SOURCE
 */

typedef struct fatfs
{
	int32_t vid; /* volume id */
	int64_t root_vnid;
  	int32_t fs_fd; /* file descriptor */
  
  	/* Filesystem globals */
	uint8_t		sectors_per_cluster;
	uint32_t	cluster_begin_lba;
	uint32_t	rootdir_first_cluster;
	uint32_t	rootdir_first_sector;
	uint32_t	rootdir_sectors;
	uint32_t	fat_begin_lba;
	uint16_t	fs_info_sector;
	uint32_t	lba_begin;
	uint32_t	fat_sectors;
	uint32_t	next_free_cluster;
	uint16_t	root_entry_count;
	tFatType	fat_type;

	/* Disk/Media API */
	struct disk_if			disk_io;

	/* Working buffer */
	struct sector_buffer	currentsector;

	/* FAT Buffer */
	struct sector_buffer	*fat_buffer_head;
	struct sector_buffer	fat_buffers[FAT_BUFFERED_SECTORS];
  
}
* fatfs_t;

/*
 ****/


extern status_t fatfs_walk (void * ns, void * base,
    char * restrict path, char ** new_path, int64_t * vnid);
extern status_t fatfs_mount (int32_t vid, const char * dev_path,
    uint32_t flags, void * params, void ** data, int64_t * p_vnid);

extern status_t fatfs_read_vnode (void * ns, int64_t vnid, void ** data);
extern status_t fatfs_write_vnode (void * ns, void * node);
extern status_t fatfs_destroy_vnode (void * ns, void * node);

extern status_t fatfs_open (void * ns, void * node, int32_t mode, void ** data);
extern status_t fatfs_close (void * ns, void * node, void * data);
extern status_t fatfs_free (void * ns, void * node, void * data);

extern status_t fatfs_create (void * ns, void * node, char * restrict path,
    int32_t mode, int32_t perms, int64_t * vnid, void ** data);
extern status_t fatfs_read (void * ns, void * node, void * file,
    void * data, int64_t offset, int32_t * p_count);
extern status_t fatfs_write (void * ns, void * node, void * file,
    void * data, int64_t offset, int32_t * p_count);

extern status_t fatfs_get_info (void * ns, void * node,
    void * data, file_info_t * p_info);
extern status_t fatfs_set_info (void * ns, void * node, void * data,
    file_info_t info, int32_t mask);

extern status_t fatfs_mkdir (void * ns, void * node,
    char * restrict name, int32_t mode);
extern status_t fatfs_readdir (void * ns, void * node, void * data,
    void * entry_array, int64_t * offset, int32_t * p_count);

extern status_t fatfs_ioctl (void * ns, void * node, void * data,
    int32_t function, va_list arguments, int32_t * p_ret);

#endif

