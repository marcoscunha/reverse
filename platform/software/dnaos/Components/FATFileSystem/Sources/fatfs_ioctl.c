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

#include <DnaTools/DnaTools.h>

/****f* FATFileSystem/fatfs_ioctl
 * SUMMARY
 * Execute an implementation-dependent operation on an inode from a FAT volume.
 *
 * SYNOPSIS
 */

status_t fatfs_ioctl (void * ns, void * node, void * data,
	int32_t function, va_list arguments, int32_t * p_ret)
    
/*
 * ARGUMENTS
 * * ns : the namespace (fatfs_t)
 * * node : the inode (fatfs_inode_t)
 * * data : the data
 * * function : the function
 * * arguments :  the arguments
 * * p_ret : the return value
 *
 * FUNCTION
 * Execute an implementation-dependent operation on an inode from a FAT volume.
 * This function is called by vfs_ioctl().
 * Not implemented.
 *
 * RESULT
 * * DNA_ERROR
 *
 * SOURCE
 */
    
{
  return DNA_ERROR;
}

/*
 ****/


