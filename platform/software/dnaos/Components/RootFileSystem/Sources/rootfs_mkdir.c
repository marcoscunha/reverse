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

status_t rootfs_mkdir (void * ns, void * node,
    char * restrict name, int32_t mode)
{
  rootfs_t rootfs = ns;
  rootfs_inode_t root_inode = node, inode = NULL;
  rootfs_entry_t dot_entry = NULL, dotdot_entry = NULL, root_entry = NULL;
  status_t status = DNA_OK;

  watch (status_t)
  {
    ensure (ns != NULL, DNA_ERROR);
    ensure (node != NULL, DNA_ERROR);
    ensure (name != NULL, DNA_ERROR);

    inode = kernel_malloc (sizeof (struct rootfs_inode), true);
    ensure (inode != NULL, DNA_OUT_OF_MEM);

    inode -> id = rootfs -> inode_index ++;
    dna_strcpy (inode -> name, name);
    inode -> class = DNA_ROOTFS_DIRECTORY;
    inode -> mode = mode;

    queue_add (& rootfs -> inode_list, inode);

    /*
     * Add the "." entry
     */

    dot_entry = kernel_malloc (sizeof (struct rootfs_entry), true);
    check (invalid_dot_entry, dot_entry != NULL, DNA_OUT_OF_MEM);

    dot_entry -> id = inode -> id;
    dna_strcpy (dot_entry -> name, ".");
    queue_add (& inode -> entry_list, dot_entry);

    /*
     * Add the ".." entry
     */

    dotdot_entry = kernel_malloc (sizeof (struct rootfs_entry), true);
    check (invalid_dotdot_entry, dotdot_entry != NULL, DNA_OUT_OF_MEM);

    dotdot_entry -> id = root_inode -> id;

    dna_strcpy (dotdot_entry -> name, "..");
    queue_add (& inode -> entry_list, dotdot_entry);

    /*
     * Add the new directory as an entry to its parent
     */

    root_entry = kernel_malloc (sizeof (struct rootfs_entry), true);
    check (invalid_root_entry, root_entry != NULL, DNA_OUT_OF_MEM);

    root_entry -> id = inode -> id;
    dna_strcpy (root_entry -> name, name);
    queue_add (& root_inode -> entry_list, root_entry);

    return status;
  }

  rescue (invalid_root_entry)
  {
    kernel_free (dotdot_entry);
  }

  rescue (invalid_dotdot_entry)
  {
    kernel_free (dot_entry);
  }

  rescue (invalid_dot_entry)
  {
    kernel_free (inode);
    leave;
  }
}

