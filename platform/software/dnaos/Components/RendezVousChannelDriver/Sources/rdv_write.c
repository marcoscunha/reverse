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
#include <Processor/Processor.h>
#include <DnaTools/DnaTools.h>

status_t rdv_write (void * handler, void * source,
    int64_t offset, int32_t * p_count)
{
  channel_rdv_t * rdv = handler;
  interrupt_status_t it_status = 0;

  it_status = cpu_trap_mask_and_backup();
  lock_acquire (& rdv -> lock);

  if (! rdv -> getter . ready)
  {
    rdv -> setter . ready = true;
    rdv -> setter . buffer = source;

    lock_release (& rdv -> lock);
    semaphore_acquire (rdv -> sem, 1, 0, -1);
  }
  else
  {
    rdv -> getter . ready = false;
    rdv -> setter . ready = false;
    dna_memcpy ((void *)rdv -> getter . buffer, (void *)source, *p_count);

    lock_release (& rdv -> lock);
    semaphore_release (rdv -> sem, 1, DNA_NO_RESCHEDULE);
  }

  cpu_trap_restore(it_status);
  return DNA_OK;
}

