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

#ifndef DEVFS_FILESYSTEM_PRIVATE_H
#define DEVFS_FILESYSTEM_PRIVATE_H

#include <stdarg.h>
#include <stdbool.h>
#include <VirtualFileSystem/VirtualFileSystem.h>
#include <DnaTools/DnaTools.h>

typedef struct devfs
{
  int64_t inode_index;
  int64_t root_vnid;
  int32_t vid;
  queue_t inode_list;
}
* devfs_t;

/*
 * VFS operations.
 */

extern status_t devfs_walk (void * ns, void * base,
    char * restrict path, char ** new_path, int64_t * vnid);
extern status_t devfs_mount (int32_t vid, const char * dev_path,
    uint32_t flags, void * params, void ** data, int64_t * p_vnid);

extern status_t devfs_read_vnode (void * ns, int64_t vnid, void ** data);
extern status_t devfs_write_vnode (void * ns, void * node);
extern status_t devfs_destroy_vnode (void * ns, void * node);

extern status_t devfs_create (void * ns, void * node, char * path,
    int32_t mode, int32_t perms, int64_t * vnid, void ** data);
extern status_t devfs_open (void * ns, void * node, int32_t mode, void ** data);
extern status_t devfs_close (void * ns, void * node, void * data);
extern status_t devfs_free (void * ns, void * node, void * data);

extern status_t devfs_read (void * ns, void * node, void * file,
    void * data, int64_t offset, int32_t * p_count);
extern status_t devfs_write (void * ns, void * node, void * file,
    void * data, int64_t offset, int32_t * p_count);

extern status_t devfs_get_info (void * ns, void * node,
    void * data, file_info_t * p_info);
extern status_t devfs_set_info (void * ns, void * node, void * data,
    file_info_t info, int32_t mask);

extern status_t devfs_mkdir (void * ns, void * node, char * name, int32_t mode);
extern status_t devfs_readdir (void * ns, void * node, void * data,
    void * entry_array, int64_t * offset, int32_t * p_count);

extern status_t devfs_ioctl (void * ns, void * node, void * data,
    int32_t function, va_list arguments, int32_t * p_ret);

#endif

