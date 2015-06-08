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

static filesystem_cmd_t fatfs_cmd = {
  fatfs_walk,
  fatfs_mount,
  fatfs_read_vnode,
  fatfs_write_vnode,
  fatfs_destroy_vnode,
  fatfs_open,
  fatfs_close,
  fatfs_free,
  fatfs_create,
  fatfs_read,
  fatfs_write,
  fatfs_get_info,
  fatfs_set_info,
  fatfs_mkdir,
  fatfs_readdir,
  fatfs_ioctl
};

filesystem_t fatfs_module = {
  "fatfs",
  & fatfs_cmd
};

