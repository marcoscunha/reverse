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

#ifndef DEVFS_INODE_PRIVATE_H
#define DEVFS_INODE_PRIVATE_H

#include <stdarg.h>
#include <stdbool.h>
#include <Private/FileSystem.h>
#include <DnaTools/DnaTools.h>

typedef enum _devfs_inode_class
{
  DNA_DEVFS_FILE,
  DNA_DEVFS_DIRECTORY,
  DNA_DEVFS_SYMLINK
}
devfs_inode_class_t;

typedef struct _devfs_inode
{
  queue_link_t link;

  int64_t id;
  devfs_inode_class_t class;
  struct _devfs_inode * parent;
  char name[DNA_FILENAME_LENGTH];

  int64_t size;
  int32_t mode;
  int32_t perms;

  device_cmd_t * dev_cmd;
  queue_t entry_list;

  char path[1];
}
* devfs_inode_t;

extern status_t devfs_inode_create (devfs_t fs, devfs_inode_t parent,
    devfs_inode_class_t class, char * name, int64_t vnid,
    device_cmd_t * commands, devfs_inode_t * p_inode);
extern status_t devfs_inode_destroy (devfs_t fs, devfs_inode_t inode);

extern status_t devfs_inode_insert (devfs_t fs, devfs_inode_t inode,
    char * path, device_cmd_t * commands);
extern status_t devfs_inode_clean (devfs_t fs, devfs_inode_t inode);

extern bool devfs_inode_id_inspector (void * inode, va_list list);

#endif

