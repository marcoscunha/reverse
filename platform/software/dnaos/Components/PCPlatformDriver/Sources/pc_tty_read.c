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

status_t pc_tty_read (void * handler, void * destination,
    int64_t offset __attribute__((unused)), int32_t * p_count)
{
  pc_tty_t * tty = (pc_tty_t *) handler;

  if (tty -> buffer . empty) semaphore_acquire (tty -> sem_id, 1, 0, -1);

  *((char *)destination) = tty -> buffer . data;
  tty -> buffer . empty = true;

  int32_t one = 1;
  pc_tty_write(handler, &tty -> buffer . data, 0, &one); 

  *p_count = 1;
  return DNA_OK;
}

