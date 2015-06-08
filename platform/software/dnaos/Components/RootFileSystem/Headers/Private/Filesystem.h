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

#ifndef ROOTFS_FILESYSTEM_PRIVATE_H
#define ROOTFS_FILESYSTEM_PRIVATE_H

#include <stdarg.h>
#include <DnaTools/DnaTools.h>
#include <VirtualFileSystem/VirtualFileSystem.h>

#define ROOTFS_NAME_LENGTH 256

typedef enum rootfs_inode_class
{
  DNA_ROOTFS_FILE,
  DNA_ROOTFS_DIRECTORY,
  DNA_ROOTFS_SYMLINK
}
rootfs_inode_class_t;

typedef struct rootfs_entry
{
  queue_link_t link;
  int64_t id;
  char name[ROOTFS_NAME_LENGTH];
}
* rootfs_entry_t;

typedef struct rootfs_inode
{
  queue_link_t link;
  int64_t id;
  char name[ROOTFS_NAME_LENGTH];
  rootfs_inode_class_t class;
  int64_t size;
  int32_t mode;
  int32_t perms;
  queue_t entry_list;
}
* rootfs_inode_t;

typedef struct rootfs
{
  int64_t inode_index;
  int64_t root_vnid;
  int32_t vid;
  queue_t inode_list;
}
* rootfs_t;

extern status_t rootfs_walk (void * ns, void * base,
    char * restrict path, char ** new_path, int64_t * vnid);
extern status_t rootfs_mount (int32_t vid, const char * dev_path,
    uint32_t flags, void * params, void ** data, int64_t * p_vnid);

extern status_t rootfs_read_vnode (void * ns, int64_t vnid, void ** data);
extern status_t rootfs_write_vnode (void * ns, void * node);
extern status_t rootfs_destroy_vnode (void * ns, void * node);

extern status_t rootfs_create (void * ns, void * node, char * path,
    int32_t mode, int32_t perms, int64_t * vnid, void ** data);
extern status_t rootfs_open (void * ns, void * node,
    int32_t mode, void ** data);
extern status_t rootfs_close (void * ns, void * node, void * data);
extern status_t rootfs_free (void * ns, void * node, void * data);

extern status_t rootfs_create (void * ns, void * node, char * restrict path,
    int32_t mode, int32_t perms, int64_t * vnid, void ** data);
extern status_t rootfs_read (void * ns, void * node, void * file,
    void * data, int64_t offset, int32_t * p_count);
extern status_t rootfs_write (void * ns, void * node, void * file,
    void * data, int64_t offset, int32_t * p_count);

extern status_t rootfs_get_info (void * ns, void * node,
    void * data, file_info_t * p_info);
extern status_t rootfs_set_info (void * ns, void * node, void * data,
    file_info_t info, int32_t mask);

extern status_t rootfs_mkdir (void * ns, void * node,
    char * restrict name, int32_t mode);
extern status_t rootfs_readdir (void * ns, void * node, void * data,
    void * entry_array, int64_t * offset, int32_t * p_count);

extern status_t rootfs_ioctl (void * ns, void * node, void * data,
    int32_t function, va_list arguments, int32_t * p_ret);

extern bool rootfs_entry_name_inspector (void * entry, va_list list);
extern bool rootfs_entry_index_inspector (void * entry, va_list list);
extern bool rootfs_inode_inspector (void * inode, va_list list);

#endif

