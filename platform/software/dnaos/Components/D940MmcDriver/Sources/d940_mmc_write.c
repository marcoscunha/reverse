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

#include <Private/Driver.h>
#include <Private/MMC.h>
#include <DnaTools/DnaTools.h>

status_t d940_mmc_write (void * handler, void * source,
    int64_t offset, int32_t * p_count)
{
  status_t status = DNA_OK;
  int64_t block_offset = offset >> 9;
  uint32_t block_register;
  int32_t block_count;
  interrupt_status_t it_status;

  watch (status_t)
  {
    ensure (source != NULL, DNA_BAD_ARGUMENT);
    ensure (*p_count != 0, DNA_BAD_ARGUMENT);
    ensure (offset % 512 == 0, DNA_BAD_ARGUMENT);
    ensure (*p_count % 512 == 0, DNA_BAD_ARGUMENT);
    ensure ((uint32_t)source % 4 == 0, DNA_BAD_ARGUMENT);

    block_count = (*p_count >> 9) + (*p_count % 512) != 0 ? 1 : 0;
    block_register = (512 << 16) | block_count;

    it_status = cpu_trap_mask_and_backup ();
    lock_acquire (& d940_mmc_driver . lock);

    check (no_card, d940_mmc_driver . card != NULL, DNA_ERROR);

    lock_acquire (& d940_mmc_driver . card -> lock);
    lock_release (& d940_mmc_driver . lock);

    cpu_write (UINT32, & (PLATFORM_MCI_BASE -> BLKR), block_register);

    status = d940_mmc_driver . mmc -> execute (d940_mmc_driver . card,
        MMC_WRITE, source, block_offset, block_count);

    lock_release (& d940_mmc_driver . card -> lock);
    cpu_trap_restore (it_status);

    return status;
  }

  rescue (no_card)
  {
    lock_release (& d940_mmc_driver . lock);
    cpu_trap_restore (it_status);

    leave;
  }
}

