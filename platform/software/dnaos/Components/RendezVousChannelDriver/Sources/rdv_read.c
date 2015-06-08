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

status_t rdv_read (void * handler, void * destination,
    int64_t offset, int32_t * p_count)
{
  channel_rdv_t * rdv = handler;
  interrupt_status_t it_status = 0;

  it_status = cpu_trap_mask_and_backup();
  lock_acquire (& rdv -> lock);
    
  if (! rdv -> setter . ready)
  {
    rdv -> getter . ready = true;
    DCACHE_FLUSH(&rdv->getter.ready, sizeof(bool));
    rdv -> getter . buffer = destination;
    DCACHE_FLUSH(&rdv->getter.buffer, sizeof(unsigned char*));
    lock_release (& rdv -> lock);
    semaphore_acquire (rdv -> sem, 1, 0, -1);
  }
  else
  {
    rdv -> setter . ready = false;
    rdv -> getter . ready = false;
    dna_memcpy ((void *)destination, (void *)rdv -> setter . buffer, *p_count);

    lock_release (& rdv -> lock);
    semaphore_release (rdv -> sem, 1, DNA_NO_RESCHEDULE);
  }

  cpu_trap_restore(it_status);
  return DNA_OK;
}

