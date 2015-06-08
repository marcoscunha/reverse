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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <Private/VirtualFileSystem.h>
#include <DnaTools/DnaTools.h>

/****v* LifeCycle/vfs_component
 * SUMMARY
 * The VFS component.
 *
 * SYNOPSIS
 */

component_t vfs_component =
{
  .name = "vfs_component",
  .create = vfs_create,
  .destroy= vfs_destroy,
  .start = vfs_start,
  .stop = vfs_stop
};

/*
 ****/

/****v* VNodePrivate/vnode_manager
 * SUMMARY
 * Instance of the vnode manager.
 *
 * SYNOPSIS
 */

vnode_manager_t vnode_manager;

/*
 ****/

/****v* VolumePrivate/volume_manager
 * SUMMARY
 * Instance of the volume manager.
 *
 * SYNOPSIS
 */

volume_manager_t volume_manager;

/*
 ****/

/****v* FilePrivate/file_pool
 * SUMMARY
 * Instance of the file pool.
 *
 * SYNOPSIS
 */

file_pool_t file_pool;

/*
 ****/

