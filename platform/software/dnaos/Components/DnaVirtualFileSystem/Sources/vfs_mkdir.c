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

#include <Private/VirtualFileSystem.h>
#include <DnaTools/DnaTools.h>
#include <Core/Core.h>

/****f* Operation/vfs_mkdir
 * SUMMARY
 * Creates a directory.
 *
 * SYNOPSIS
 */

status_t vfs_mkdir (char * restrict path, int32_t mode)

/*
 * ARGUMENTS
 * * path : path of the directory where to mkdir.
 * * mode : mkdir mode.
 *
 * RESULT
 * * DNA_NO_ENTRY if the path is not valid.
 * * DNA_ERROR if an error occured.
 * * DNA_OK if the operation succeeded.
 *
 * SOURCE
 */

{
  status_t status = DNA_OK;
  int64_t vnid = -1;
  volume_t volume = NULL;
  void * data = NULL;
  char buffer[DNA_PATH_LENGTH], token[DNA_FILENAME_LENGTH];

  watch (status_t)
  {
    ensure (path != NULL, DNA_ERROR);

    dna_strcpy (buffer, path);

    status = path_split (buffer, token);
    ensure (status == DNA_OK, status);

    status = vnode_walk (buffer, & volume, & vnid, & data);
    ensure (status == DNA_OK, status);

    status = volume -> cmd -> mkdir (volume -> data, data, token, mode);
    ensure (status == DNA_OK, status);

    status = vnode_put (volume -> id, vnid);
    ensure (status == DNA_OK, status);

    return DNA_OK;
  }
}

/*
 ****/

