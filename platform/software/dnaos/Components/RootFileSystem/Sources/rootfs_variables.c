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
#include <DnaTools/DnaTools.h>

static filesystem_cmd_t rootfs_cmd =
{
  rootfs_walk,
  rootfs_mount,
  rootfs_read_vnode,
  rootfs_write_vnode,
  rootfs_destroy_vnode,
  rootfs_open,
  rootfs_close,
  rootfs_free,
  rootfs_create,
  rootfs_read,
  rootfs_write,
  rootfs_get_info,
  rootfs_set_info,
  rootfs_mkdir,
  rootfs_readdir,
  rootfs_ioctl
};

filesystem_t rootfs_module =
{
  "rootfs",
  & rootfs_cmd
};

