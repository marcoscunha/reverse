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

#include <Private/Filesystem.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

status_t rootfs_create (void * ns, void * node, char * restrict path,
    int32_t mode, int32_t perms, int64_t * vnid, void ** data)
{
  rootfs_t rootfs = ns;
  rootfs_inode_t root_inode = node, inode = NULL;
  rootfs_entry_t entry = NULL;

  inode = kernel_malloc (sizeof (struct rootfs_inode), true);
  if (inode == NULL) return DNA_OUT_OF_MEM;
  inode -> id = rootfs -> inode_index ++;
  dna_strcpy (inode -> name, path);
  inode -> class = DNA_ROOTFS_FILE;
  inode -> perms = perms;
  inode -> mode = mode;
  queue_add (& rootfs -> inode_list, inode);

  /*
   * Add the new directory as an entry to its parent
   */

  entry = kernel_malloc (sizeof (struct rootfs_entry), true);
  if (entry == NULL) return DNA_OUT_OF_MEM;
  entry -> id = inode -> id;
  dna_strcpy (entry -> name, path);
  queue_add (& root_inode -> entry_list, entry);

  /*
   * Add the root vnode to the system
   */

  *vnid = inode -> id;
  *data = NULL;

  return vnode_create (inode -> id, rootfs -> vid, (void *) inode);
}

