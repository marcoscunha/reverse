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

int32_t soclib_tty_isr (void * data)
{
  int32_t index = 0, status, c;

  for (index = 0; index < SOCLIB_TTY_NDEV; index += 1)
  {
    cpu_read (UINT32, & (TTY[index] . port -> status), status);

    if (status != 0)
    {
      cpu_read (UINT32, & (TTY[index] . port -> read), c);

      if (TTY[index] . buffer . empty)
      {
        TTY[index] . buffer . empty = false;
      }

      TTY[index] . buffer . data = c;

      semaphore_release (TTY[index] . sem_id, 1, DNA_NO_RESCHEDULE);
      return DNA_INVOKE_SCHEDULER;
    }
  }

  return DNA_UNHANDLED_INTERRUPT;
}

