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

#include <Private/DeviceFileSystem.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

status_t devfs_mount (int32_t vid, const char * dev_path, uint32_t flags,
    void * params, void ** data, int64_t * vnid)
{
  status_t status = DNA_OK;
  devfs_t devfs = NULL;
  devfs_inode_t root_inode = NULL;
  int32_t driver_index = 0;

  watch (status_t)
  {
    /*
     * Initialize the DevFS structure.
     */

    devfs = kernel_malloc (sizeof (struct devfs), true);
    ensure (devfs != NULL, DNA_OUT_OF_MEM);

    devfs -> inode_index = 1;
    devfs -> vid = vid;
    devfs -> root_vnid = devfs -> inode_index ++;

    /*
     * Initialize the root inode structure.
     */

    status = devfs_inode_create (devfs, NULL, DNA_DEVFS_DIRECTORY,
        "", devfs -> root_vnid, NULL, & root_inode);
    check (inode_error, status == DNA_OK, status);

    status = devfs_entry_add (root_inode, ".", root_inode -> id);
    check (entry_error, status == DNA_OK, status);

    status = devfs_entry_add (root_inode, "..", root_inode -> id);
    check (entry_error, status == DNA_OK, status);

    /*
     * Load the in-kernel drivers and check if they are consistent.
     */

    for (driver_index = 0; driver_index < OS_N_DRIVERS; driver_index ++)
    {
      status = OS_DRIVERS_LIST[driver_index] -> init_hardware ();
      check (bad_driver, status == DNA_OK, status);

      status = OS_DRIVERS_LIST[driver_index] -> init_driver ();
      check (bad_driver, status == DNA_OK, status);
    }

    /*
     * Add the root vnode to the system
     */

    *data = devfs;
    DCACHE_FLUSH(data, sizeof(devfs_t));
    *vnid = root_inode -> id;

    dna_log(VERBOSE_LEVEL, "DevFS mount succeeded.");
    return vnode_create (root_inode -> id, devfs -> vid, (void *) root_inode);
  }

  rescue (bad_driver)
  {
    for (int32_t i = 0; i < driver_index; i += 1)
    {
      OS_DRIVERS_LIST[i] -> uninit_driver ();
    }

    devfs_entry_t entry = queue_rem (& root_inode -> entry_list);
    kernel_free (entry);
  }

  rescue (entry_error)
  {
    devfs_inode_destroy (devfs, root_inode);
  }

  rescue (inode_error)
  {
    kernel_free (devfs);
    leave;
  }
}

