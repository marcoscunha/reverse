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

#include <Private/DBGU.h>
#include <Private/Driver.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

int32_t d940_dbgu_isr (void * data)
{
	uint32_t status = 0, value = 0;

	cpu_read (UINT32, & (d940_dbgu_driver . port -> SR), status);

	if (status & 0x1)
  {
    log (INFO_LEVEL, "got write interrupt.");
		cpu_read (UINT32, & (d940_dbgu_driver . port -> RHR), value);
		cpu_write (UINT32, & (d940_dbgu_driver . port -> THR), value);
		return DNA_HANDLED_INTERRUPT;
	}

  return DNA_UNHANDLED_INTERRUPT;
}

