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
#include <Processor/Processor.h>

status_t mbr_load (uint8_t sector[512], mbr_partition_t partition[4])
{
  int32_t valid_count = 0;
  uint16_t signature;
  mbr_t * mbr = (mbr_t *)sector;

  watch (status_t)
  {
    ensure (sector != NULL, DNA_BAD_ARGUMENT);
    ensure (partition != NULL, DNA_BAD_ARGUMENT);
    
    signature = mbr -> fields . signature;
    cpu_data_is_big_endian (16, signature);
    ensure (signature == 0x55AA, DNA_ERROR);

    dna_memset (partition, 0, 4 * sizeof (mbr_partition_t));

    for (int32_t i = 0; i < 4; i += 1)
    {
      partition[i] = mbr -> fields . partition[i];
      ensure (partition[i] . status == MBR_NORMAL_PARTITION ||
          partition[i] . status == MBR_BOOTABLE_PARTITION, DNA_ERROR);

      if (mbr -> fields . partition[i] . type != 0)
      {
        valid_count += 1;
        partition[i] = mbr -> fields . partition[i];
      }
    }

    ensure (valid_count > 0, DNA_ERROR);
    return DNA_OK;
  }
}
