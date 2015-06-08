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

#include <fcntl.h>
#include <Private/Media.h>

/****m* FATFileSystem/media_open
 * SUMMARY
 * Return a file descriptor to a FAT device.
 *
 * SYNOPSIS
 */

int media_open(const char *dev_path, int32_t *fs_fd)

/*  
 * ARGUMENTS
 * * dev_path : the path of the FAT device.
 * * fs_fd : the file_descriptor
 * 	
 * FUNCTION
 * Return a file descriptor to a FAT device.
 *
 * RESULT
 * * 1 : on success
 * * 0 : on faillure
 *
 * SOURCE
 */

{
	return ((*fs_fd = open(dev_path, O_RDWR)) >= 0); 
}

/*
 ****/

