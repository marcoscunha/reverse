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

#include <Private/FATFileSystem.h>
#include <DnaTools/DnaTools.h>

/****f* FATFileSystem/fatfs_create
 * SUMMARY
 * Creates an inode on a FAT volume.
 *
 * SYNOPSIS
 */

status_t fatfs_create (void * ns, void * node, char * restrict path,
    int32_t mode, int32_t perms, int64_t * vnid, void ** data)
    
/*
 * ARGUMENTS
 * * ns : the namespace (fatfs_t)
 * * node : the base inode (fatfs_inode_t)
 * * path : the path
 * * mode : unused
 * * perms : unused
 * * vnid : the new inode id
 * * data : the content of the new inode (fatfs_inode_t)
 *
 * FUNCTION
 * Creates an inode on a FAT volume.
 * Function called by vnode_create().
 * Not implemented.
 *
 * RESULT
 * * DNA_OK
 *
 * SOURCE
 */
    
{
  return DNA_OK;
}

/*
 ****/

