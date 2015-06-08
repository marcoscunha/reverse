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
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

/****f* VolumePrivate/volume_destroy
 * SUMMARY
 * Destroy a volume.
 *
 * SYNOPSIS
 */

status_t volume_destroy (volume_t volume)

/*
 * ARGUMENTS
 * * volume : the volume to destroy.
 *
 *
 * RESULT
 *
 * SOURCE
 */

{
  interrupt_status_t it_status = 0;

  it_status = cpu_trap_mask_and_backup();
  lock_acquire (& volume_manager . volume_list . lock);

  queue_extract (& volume_manager . volume_list, volume);

  lock_release (& volume_manager . volume_list . lock);
  cpu_trap_restore(it_status);

  kernel_free (volume);
  return DNA_OK;
}

/*
 ****/

