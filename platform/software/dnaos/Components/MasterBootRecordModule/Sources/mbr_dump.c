/*
 * Copyright (C 2007 TIMA Laboratory                                    
 *
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or     
 * (at your option any later version.                                   
 *                                                                       
 * This program is distributed in the hope that it will be useful,       
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
 * GNU General Public License for more details.                          
 *                                                                       
 * You should have received a copy of the GNU General Public License     
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include <stdint.h>
#include <Private/MasterBootRecord.h>
#include <DnaTools/DnaTools.h>

status_t mbr_dump (mbr_partition_t partition[4], uint8_t sector[512])
{
  int32_t next_partition = 0;
  uint16_t signature = 0x55AA;
  mbr_t * mbr = (mbr_t *)sector;

  watch (status_t)
  {
    ensure (sector != NULL, DNA_BAD_ARGUMENT);
    ensure (partition != NULL, DNA_BAD_ARGUMENT);
    
    dna_memset (sector, 0, 512);
    
    cpu_data_is_big_endian (16, signature);
    mbr -> fields . signature = signature;

    for (int32_t i = 0; i < 4; i += 1)
    {
      if (partition[i] . type != 0)
      {
        ensure (partition[i] . status == MBR_NORMAL_PARTITION ||
            partition[i] . status == MBR_BOOTABLE_PARTITION, DNA_ERROR);

        mbr -> fields . partition[next_partition] = partition[i];
        next_partition += 1;
      }
    }

    return DNA_OK;
  }
}
