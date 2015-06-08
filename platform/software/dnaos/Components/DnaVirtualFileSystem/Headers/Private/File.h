/****h* VirtualFileSystem/FilePrivate
 * SUMMARY
 * VFS File private header.
 ****
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

#ifndef DNA_VFS_FILE_PRIVATE_H
#define DNA_VFS_FILE_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>
#include <Private/VNode.h>
#include <DnaTools/DnaTools.h>

/****t* FilePrivate/file_t
 * SUMMARY
 * File type.
 *
 * SOURCE
 */

typedef struct _file
{
#ifdef  WRITEBACK
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare1[7];
  int32_t usage_counter __attribute__((aligned(32)));
  int32_t spare2[7];
#else
  spinlock_t lock;
  int32_t usage_counter;
#endif

  vnode_t vnode;

  bool destroy;
  int32_t mode;
  int64_t offset;
//#ifdef WRITEBACK
  struct _file* kalloc_addr;
//#endif
  void * data;
}
* file_t;

/*
 ****/

/****t* FilePrivate/file_manager_t
 * SUMMARY
 * File manager type.
 *
 * SOURCE
 */

typedef struct _file_pool
{
#ifdef  WRITEBACK
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare[7];
#else
  spinlock_t lock;
#endif
  file_t ** file;
}
file_pool_t;

/*
 ****/

extern file_pool_t file_pool;

extern status_t file_create (vnode_t vnode, int32_t mode, void * data,
    int16_t * fd, file_t * p_file);
extern status_t file_destroy (int16_t fd);
extern status_t file_get (int16_t fd, file_t * p_file);
extern status_t file_put (int16_t fd);

#endif
