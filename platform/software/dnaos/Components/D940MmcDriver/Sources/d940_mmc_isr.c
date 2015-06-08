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
#include <Private/MMC.h>
#include <DnaTools/DnaTools.h>

int32_t d940_mmc_isr (void * data)
{
  uint32_t status_reg = 0, int_mask = 0;

  cpu_read (UINT32, & (PLATFORM_MCI_BASE -> SR), status_reg);
  cpu_read (UINT32, & (PLATFORM_MCI_BASE -> IMR), int_mask);

  log (INFO_LEVEL, "Status reg = 0x%x, mask = 0x%x", status_reg, int_mask);

  return 0;
}

