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

/****f* FATFileSystem/fatfs_mkdir
 * SUMMARY
 * Make a new directory on a FAT volume..
 *
 * SYNOPSIS
 */

status_t fatfs_mkdir (void * ns, void * node,
    char * restrict name, int32_t mode)
    
/*  
 * ARGUMENTS
 * * ns : the namespace (fatfs_t)
 * * node : the base inode containing the directory to make (fatfs_inode_t)
 * * name : name of the directory to make
 * * mode : unused
 *
 * FUNCTION
 * Make a new directory on a FAT volume.
 * This function is called by vfs_mkdir().
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


