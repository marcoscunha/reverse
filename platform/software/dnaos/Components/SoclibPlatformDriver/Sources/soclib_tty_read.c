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
#include <DnaTools/DnaTools.h>

status_t soclib_tty_read (void * handler, void * destination,
    int64_t offset, int32_t * p_count)
{
  soclib_tty_t * tty = (soclib_tty_t *) handler;

  if (tty -> buffer . empty) semaphore_acquire (tty -> sem_id, 1, 0, -1);

  *((char *)destination) = tty -> buffer . data;
  tty -> buffer . empty = true;

  cpu_write (UINT8, (& tty -> port -> write), tty -> buffer . data); // echoing typed character

  *p_count = 1;
  return DNA_OK;
}

