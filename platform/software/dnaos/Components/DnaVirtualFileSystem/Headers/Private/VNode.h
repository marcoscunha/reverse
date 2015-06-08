/****h* VirtualFileSystem/VNodePrivate
 * SUMMARY
 * vNode management, private header.
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

#ifndef DNA_VFS_VNODE_H
#define DNA_VFS_VNODE_H

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <Private/Volume.h>

#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

/****t* VNodePrivate/vnode_t
 * SUMMARY
 * Virtual node type.
 *
 * SOURCE
 */

typedef struct _vnode
{
  queue_link_t link;

  int64_t id;
  volume_t volume;
  bool destroy;
#ifdef WRITEBACK 
  int32_t usage_counter __attribute__((aligned(32)));
  int32_t spare[7];
#else
  int32_t usage_counter;
#endif
  void * data;
  #if 1 
  struct _vnode* kalloc_addr;
  #endif
}
* vnode_t;

/*
 ****/

/****t* VNodePrivate/vnode_manager_t
 * SUMMARY
 * Vnode manager type.
 *
 * SOURCE
 */

typedef struct _vnode_manager
{
  #ifdef WRITEBACK
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare[7];
  #else
  spinlock_t lock;
  #endif

  queue_t vnode_list;
}
vnode_manager_t;

/*
 ****/

extern vnode_manager_t vnode_manager;

extern status_t vnode_create (int64_t vnid, int32_t vid, void * data);
extern status_t vnode_destroy (int32_t vid, int64_t vnid);

extern status_t vnode_walk (char * restrict path, volume_t * p_volume,
    int64_t * p_vnid, void ** p_data);
extern status_t vnode_get (int32_t nsid, int64_t vnid, void ** data);
extern status_t vnode_put (int32_t nsid, int64_t vnid);

extern bool vnode_id_inspector (void * node, va_list list);
extern bool vnode_volume_inspector (void * node, va_list list);

#endif
