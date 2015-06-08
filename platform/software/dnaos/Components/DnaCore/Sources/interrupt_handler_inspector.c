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

#include <Private/Core.h>
#include <DnaTools/DnaTools.h>

/****f* InterruptPrivate/interrupt_handler_inspector
 * SUMMARY
 * Compare an handler with an ISR handler.
 *
 * SYNOPSIS
 */

bool interrupt_handler_inspector (void * isr, va_list list)

/*
 * ARGUMENTS
 * * isr : an ISR element
 * * list : a list of variable arguments
 *
 * FUNCTION
 * Compare handler and isr -> handler.
 *
 * RESULT
 * Return TRUE if they match, FALSE otherwise.
 *
 * SOURCE
 */

{
  isr_t service = isr;
  interrupt_handler_t handler = va_arg (list, interrupt_handler_t);

  watch (bool)
  {
    ensure (isr != NULL, false);
    ensure (handler != NULL, false);

    return service -> handler == (interrupt_handler_t)handler;
  }
}

/*
 ****/
