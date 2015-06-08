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
#include <Processor/Processor.h>
#include <Platform/Platform.h>

int32_t soclib_ipi_isr (void * data)
{
  int32_t value, command, status;
  int32_t current_cpuid = cpu_mp_id ();

  cpu_read (UINT32, & (PLATFORM_IPI_BASE[current_cpuid] . data), value);
  cpu_read (UINT32, & (PLATFORM_IPI_BASE[current_cpuid] . command), command);

  cpu_read (UINT32, & (PLATFORM_IPI_BASE[current_cpuid] . reset), status);
  
  if (status != 0)
  {
    cpu_write (UINT32, & (PLATFORM_IPI_BASE[current_cpuid] . reset), 0);

    switch (ipi_handler (command, (void *)value))
    {
      case DNA_OK :
        return DNA_HANDLED_INTERRUPT;

      case DNA_INVOKE_SCHEDULER :
        return DNA_INVOKE_SCHEDULER;

      default :
        return DNA_UNHANDLED_INTERRUPT;
    }
  }

  return DNA_UNHANDLED_INTERRUPT;
}

