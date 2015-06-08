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

#include <Private/Status.h>
#include <Private/LowLevelOperations.h>
#include <DnaTools/DnaTools.h>
#include <Platform/MCI.h>

status_t d940_mmc_read_low (void * buffer, int32_t word_count)
{
  d940_mmc_status_t status;

  for (int32_t i = 0; i < word_count; i += 1)
  {
    do
    {
      cpu_read (UINT32, & (PLATFORM_MCI_BASE -> SR), status . raw);
    }
    while (status . bits . receiver_ready == 0);

    cpu_read (UINT32, & (PLATFORM_MCI_BASE -> RDR),
        ((uint32_t *)buffer)[i]);
  }

  return DNA_OK;
}

