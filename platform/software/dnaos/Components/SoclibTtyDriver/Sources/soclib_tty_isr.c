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

#include <Private/Soclib.h>
#include <DnaTools/DnaTools.h>

int32_t soclib_tty_isr (int32_t itn)
{
  int32_t index = 0;

  for (index = 0; index < SOCLIB_TTY_NDEV; index += 1)
  {
    if (TTY[index] . irq == itn) break;
  }

  if (index == SOCLIB_TTY_NDEV)
  {
    return DNA_UNHANDLED_INTERRUPT;
  }
  else
  {
    TTY[index] . buffer . data = TTY[index] . port -> read; 

    if (TTY[index] . buffer . empty)
    {
      TTY[index] . buffer . empty = false;
    }

    /*
     * Releasing the semaphore without further check
     * is dangerous
     */

    semaphore_release (TTY[index] . sem_id, 1, 0);
    return DNA_HANDLED_INTERRUPT;
  }
}

