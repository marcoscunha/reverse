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

#include <Private/CSD.h>
#include <DnaTools/DnaTools.h>

void mmc_build_csd (uint32_t response[4], mmc_csd_t * csd)
{
#if 0
  for (int32_t i = 0; i < 4; i += 1)
  {
    dna_log(VERBOSE_LEVEL, "CSD[%d] 0x%x", i, response[i]);
  }

  for (int32_t i = 0; i < 4; i += 1)
  {
    dna_log(VERBOSE_LEVEL, "CSD[%d] 0x%x 0x%x 0x%x 0x%x", i,
        ((char *)response)[i * 4 + 0],
        ((char *)response)[i * 4 + 1],
        ((char *)response)[i * 4 + 2],
        ((char *)response)[i * 4 + 3]);
  }
#endif

  csd -> csd_structure = CSD_STRUCTURE(response[0]);

  csd -> read_access_time = READ_ACCESS_TIME(response[0]);
  csd -> read_access_cycles = READ_ACCESS_CYCLES(response[0]);
  csd -> data_transfer_rate = DATA_TRANSFER_RATE(response[0]);

  csd -> command_classes = COMMAND_CLASSES(response[1]);
  csd -> read_block_length = READ_BLOCK_LENGTH(response[1]);
  csd -> read_block_partial = READ_BLOCK_PARTIAL(response[1]);
  csd -> write_block_misalign = WRITE_BLOCK_MISALIGN(response[1]);
  csd -> read_block_misalign = READ_BLOCK_MISALIGN(response[1]);
  csd -> dsr_implemented = DSR_IMPLEMENTED(response[1]);

  switch (csd -> csd_structure)
  {
    case CSD_V1_STRUCTURE :
      csd -> device_size = DEVICE_SIZE_V1(response[1], response[2]);
      csd -> read_vdd_min = READ_VDD_MIN(response[2]);
      csd -> read_vdd_max = READ_VDD_MAX(response[2]);
      csd -> write_vdd_min = WRITE_VDD_MIN(response[2]);
      csd -> write_vdd_max = WRITE_VDD_MAX(response[2]);
      csd -> size_multiplier = SIZE_MULTIPLIER(response[2]);

      break;

    case CSD_V2_STRUCTURE :
      csd -> device_size = DEVICE_SIZE_V2(response[1], response[2]);
      csd -> read_vdd_min = 0;
      csd -> read_vdd_max = 0;
      csd -> write_vdd_min = 0;
      csd -> write_vdd_max = 0;
      csd -> size_multiplier = 0;

      break;
  }

  csd -> erase_block_enable = ERASE_BLOCK_ENABLE(response[2]);
  csd -> erase_sector_size = ERASE_SECTOR_SIZE(response[2]);
  csd -> wp_group_size = WP_GROUP_SIZE(response[2]);

  csd -> wp_group_enable = WP_GROUP_ENABLE(response[3]);
  csd -> write_speed_factor = WRITE_BLOCK_LENGTH(response[3]);
  csd -> write_block_length = WRITE_BLOCK_LENGTH(response[3]);
  csd -> write_block_partial = WRITE_BLOCK_PARTIAL(response[3]);
  csd -> file_format_group = FILE_FORMAT_GROUP(response[3]);
  csd -> copy = COPY_FLAG(response[3]);
  csd -> permanent_write_protect = PERM_WRITE_PROTECTION(response[3]);
  csd -> temporary_write_protect = TEMP_WRITE_PROTECTION(response[3]);
  csd -> file_format = FILE_FORMAT(response[3]);
  csd -> crc = CRC(response[3]);
  csd -> always_one = ALWAYS_ONE(response[3]);
}

