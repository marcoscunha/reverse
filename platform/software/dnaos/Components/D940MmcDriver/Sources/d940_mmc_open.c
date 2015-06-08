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

status_t d940_mmc_open (char * name, int32_t mode, void ** data)
{
  watch (status_t)
  {
    ensure (d940_mmc_driver . card != NULL, DNA_ERROR);
    ensure (dna_strcmp (name, "disk/mmc/0/raw") == 0, DNA_ERROR);

    *data = (void *) d940_mmc_driver . card;
    return DNA_OK;
  }
}

