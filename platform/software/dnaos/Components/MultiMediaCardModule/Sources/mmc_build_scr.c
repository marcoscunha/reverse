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

#include <Private/SCR.h>
#include <DnaTools/DnaTools.h>

void mmc_build_scr (uint32_t response[4], mmc_scr_t * scr)
{
  scr -> scr_structure = SCR_STRUCTURE(response[0]);
  scr -> sd_specification = SD_SPECIFICATION(response[0]);
  scr -> data_status_after_erase = DATA_STATUS_AFTER_ERASE(response[0]);
  scr -> security_support = SD_SECURITY(response[0]);
  scr -> data_bus_width = SD_BUS_WIDTH(response[0]);
}

