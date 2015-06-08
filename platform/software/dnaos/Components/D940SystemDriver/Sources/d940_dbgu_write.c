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

status_t d940_dbgu_write (void * handler, void * source,
    int64_t offset, int32_t * p_count)
{
	d940_dbgu_driver_t * dbgu = (d940_dbgu_driver_t *) handler;
	uint32_t res = 0;

  for (int32_t i = 0; i < *p_count; i += 1)
  {
    do cpu_read (UINT32, & (dbgu -> port -> SR), res);
    while ((res & 0x202) == 0);

    cpu_write (UINT32, & (dbgu -> port -> THR), ((char *)source)[i]);
  }

  return DNA_OK;
}

