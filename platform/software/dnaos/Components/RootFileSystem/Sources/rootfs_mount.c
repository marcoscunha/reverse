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

status_t rootfs_mount (int32_t vid, const char * dev_path, uint32_t flags,
    void * params, void ** data, int64_t * vnid)
{
  rootfs_t rootfs = NULL;
  rootfs_inode_t root_inode = NULL;
  rootfs_entry_t entry = NULL;
  char * entry_name[] = { ".", "..", "devices", "volumes", "system" };

   rootfs = kernel_malloc (sizeof (struct rootfs), true);
  if (rootfs == NULL) return DNA_OUT_OF_MEM;

   root_inode = kernel_malloc (sizeof (struct rootfs_inode), true);
  if (root_inode == NULL) return DNA_OUT_OF_MEM;

  rootfs -> inode_index = 1;
  rootfs -> vid = vid;

  root_inode -> id = rootfs -> inode_index ++;

  dna_strcpy (root_inode -> name, "");
  root_inode -> class = DNA_ROOTFS_DIRECTORY;
  queue_add (& rootfs -> inode_list, root_inode);

  rootfs -> root_vnid = root_inode -> id;

  /*
   * Add the "." and ".." entry
   */

  for (int32_t i = 0; i < 2; i ++)
  {
    entry = kernel_malloc (sizeof (struct rootfs_entry), true);
    if (entry == NULL) return DNA_OUT_OF_MEM;

    entry -> id = root_inode -> id;
    dna_strcpy (entry -> name, entry_name[i]);
    queue_add (& root_inode -> entry_list, entry);
  }

  /*
   * Create the standard directories
   */

  for (int32_t i = 2; i < 5; i++)
  {
    rootfs_mkdir (rootfs, root_inode, entry_name[i], 0);
  }

  /*
   * Add the root vnode to the system
   */

  *data = rootfs;
  *vnid = root_inode -> id;

  dna_log(VERBOSE_LEVEL, "VNODE CREATE ID 0x%x, VID 0x%x",
      root_inode -> id, rootfs -> vid);

  return vnode_create (root_inode -> id, rootfs -> vid, (void *) root_inode);
}

