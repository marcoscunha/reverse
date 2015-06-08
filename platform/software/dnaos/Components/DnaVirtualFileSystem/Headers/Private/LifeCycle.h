/****h* VirtualFileSystem/LifeCycle
 * SUMMARY
 * VFS life cycle.
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

#ifndef DNA_VFS_LIFECYCLE_PRIVATE_H
#define DNA_VFS_LIFECYCLE_PRIVATE_H

#include <DnaTools/DnaTools.h>

extern status_t vfs_create (void);
extern status_t vfs_destroy (void);
extern status_t vfs_start (void);
extern status_t vfs_stop (void);

#endif

