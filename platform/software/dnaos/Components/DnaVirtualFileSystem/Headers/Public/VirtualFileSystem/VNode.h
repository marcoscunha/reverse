/****h* VirtualFileSystem/VNode
 * SUMMARY
 * vNode management.
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

#ifndef DNA_VFS_VNODE_PUBLIC_H
#define DNA_VFS_VNODE_PUBLIC_H

#include <stdint.h>
#include <DnaTools/DnaTools.h>

extern status_t vnode_create (int64_t vnid, int32_t vid, void * data);
extern status_t vnode_destroy (int32_t vid, int64_t vnid);

extern status_t vnode_get (int32_t nsid, int64_t vnid, void ** data);
extern status_t vnode_put (int32_t nsid, int64_t vnid);

#endif
