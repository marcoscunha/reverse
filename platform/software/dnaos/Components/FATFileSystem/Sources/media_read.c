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

#include <unistd.h>
#include <Private/FATFileSystem.h>
#include <Private/Media.h>

/****m* FATFileSystem/media_read
 * SUMMARY
 * Perform the reading of one sector on a FAT device.
 *
 * SYNOPSIS
 */

int media_read(const uint32_t sector, unsigned char *buffer, 
	const int32_t fs_fd)

/*  
 * ARGUMENTS
 * * sector : the sector to read
 * * buffer : the read data
 * * fs_fd : the file descriptor pointing to the FAT volume.
 * 	
 * FUNCTION
 * Perform the reading of one sector on a FAT device.
 *
 * RESULT
 * * 1 : on success
 * * 0 : on faillure
 *
 * SOURCE
 */

{
	if(lseek(fs_fd, (sector * FAT_SECTOR_SIZE), SEEK_SET) < 0)
		return 0;

	return (read(fs_fd, buffer, FAT_SECTOR_SIZE) > 0);
}

/*
 ****/


