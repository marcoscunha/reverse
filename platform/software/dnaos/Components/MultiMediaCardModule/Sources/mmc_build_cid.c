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

void mmc_build_cid (uint32_t response[4], mmc_cid_t * cid)
{
#if 0
  for (int32_t i = 0; i < 4; i += 1)
  {
    dna_log(VERBOSE_LEVEL, "CID[%d] 0x%x", i, response[i]);
  }

  for (int32_t i = 0; i < 4; i += 1)
  {
    dna_log(VERBOSE_LEVEL, "CID[%d] 0x%x 0x%x 0x%x 0x%x", i,
        ((char *)response)[i * 4 + 0],
        ((char *)response)[i * 4 + 1],
        ((char *)response)[i * 4 + 2],
        ((char *)response)[i * 4 + 3]);
  }
#endif

  cid -> manufacturer_id = MANUFACTURER_ID(response[0]);
  cid -> oem_id = OEM_ID(response[0]);

  cid -> name[0] = PRODUCT_NAME_C1(response[0]);
  cid -> name[1] = PRODUCT_NAME_C2(response[1]);
  cid -> name[2] = PRODUCT_NAME_C3(response[1]);
  cid -> name[3] = PRODUCT_NAME_C4(response[1]);
  cid -> name[4] = PRODUCT_NAME_C5(response[1]);
  cid -> name[5] = '\0';

  cid -> revision = PRODUCT_REVISION(response[2]);
  cid -> serial_number = SERIAL_NUMBER(response[2], response[3]);
  cid -> manufacturing_date = MANUFACTURING_DATE(response[3]);
}

