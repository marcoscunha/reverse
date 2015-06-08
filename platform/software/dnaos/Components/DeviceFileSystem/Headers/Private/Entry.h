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

#ifndef DEVFS_ENTRY_PRIVATE_H
#define DEVFS_ENTRY_PRIVATE_H

#include <stdarg.h>
#include <stdbool.h>
#include <Private/iNode.h>
#include <DnaTools/DnaTools.h>

typedef struct devfs_entry
{
  queue_link_t link;
  int64_t id;
  bool loaded;
  char name[DNA_FILENAME_LENGTH];
}
* devfs_entry_t;

extern status_t devfs_entry_add (devfs_inode_t inode,
    char * name, int64_t vnid);
extern status_t devfs_entry_remove_by_name (devfs_inode_t inode, char * name);
extern status_t devfs_entry_remove_by_index (devfs_inode_t inode,
    int32_t index);

extern bool devfs_entry_name_inspector (void * entry, va_list list);
extern bool devfs_entry_index_inspector (void * entry, va_list list);
extern bool devfs_entry_unused_inspector (void * entry, va_list list);

#endif

