/****h* VirtualFileSystem/VolumePrivate
 * SUMMARY
 * Volume management, private header.
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

#ifndef DNA_VFS_VOLUME_H
#define DNA_VFS_VOLUME_H

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <VirtualFileSystem/Filesystem.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

/****t* VolumePrivate/volume_t
 * SUMMARY
 * Name space type.
 *
 * SOURCE
 */

typedef struct _volume
{
  queue_link_t link;

  int32_t id;
  int64_t root_vnid;

#ifdef WRITEBACK
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare[7];
#else
  spinlock_t lock;
#endif

  struct _volume * host_volume;
  int64_t host_vnid;

  filesystem_cmd_t * cmd;
  void * data;
}
* volume_t;

/*
 ****/

/****t* VolumePrivate/volume_manager_t
 * SUMMARY
 * Volume manager type.
 *
 * SOURCE
 */

typedef struct _volume_manager
{
#ifdef WRITEBACK
  spinlock_t lock __attribute__((aligned(32))); 
  int32_t spare1[7];
  int32_t volume_index __attribute__((aligned(32))); 
;
  int32_t spare2[7];
#else
  spinlock_t lock;
  int32_t volume_index;
#endif
  queue_t volume_list;
}
volume_manager_t;

/*
 ****/

extern volume_manager_t volume_manager;

extern status_t volume_create (volume_t host_volume, int64_t host_vnid,
    filesystem_cmd_t * cmd, volume_t * p_volume);
extern status_t volume_destroy (volume_t volume);

extern bool volume_id_inspector (void * item, va_list list);
extern bool volume_host_inspector (void * item, va_list list);

#endif

