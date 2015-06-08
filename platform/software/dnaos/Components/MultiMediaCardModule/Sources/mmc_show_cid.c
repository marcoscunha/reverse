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

#include <Private/CID.h>
#include <DnaTools/DnaTools.h>

void mmc_show_cid (mmc_cid_t cid)
{
  dna_printf ("Manufacturer ID: 0x%x\r\n", cid . manufacturer_id);
  dna_printf ("OEM ID: 0x%x\r\n", cid . oem_id);
  dna_printf ("Product name : %s\r\n", cid . name);
  dna_printf ("Revision: 0x%x\r\n", cid . revision);
  dna_printf ("S/N: 0x%x\r\n", cid . serial_number);
  dna_printf ("Date: 0x%x\r\n", cid . manufacturing_date);
}

