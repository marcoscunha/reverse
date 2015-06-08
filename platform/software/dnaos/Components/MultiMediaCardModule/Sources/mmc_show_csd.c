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

void mmc_show_csd (mmc_csd_t csd)
{
  uint32_t memory_size = 0, blk_len, mult, blk_nr;

  dna_printf ("CSD structure = %d\r\n", csd . csd_structure);
  dna_printf ("Read block length = 0x%x\r\n", csd . read_block_length);
  dna_printf ("Device size = 0x%x\r\n", csd . device_size);

  switch (csd . csd_structure)
  {
    case CSD_V1_STRUCTURE :
      dna_log(INFO_LEVEL, "Size mult = 0x%x", csd . size_multiplier);

      mult = 1 << (csd . size_multiplier + 2);
      blk_len = 1 << csd . read_block_length;
      blk_nr = (csd . device_size + 1) * mult;
      memory_size = blk_nr * blk_len;
      break;

    case CSD_V2_STRUCTURE :
      memory_size = (csd . device_size + 1) * 512 * 1024;

      break;
  }

  dna_printf ("Memory size = %d bytes (%d MB)\r\n",
      memory_size, memory_size / (1024 * 1024));

#if 0
  csd -> read_access_time        = READ_ACCESS_TIME(rsp . raw[0]);
  csd -> read_access_cycles      = READ_ACCESS_CYCLES(rsp . raw[0]);
  csd -> data_transfer_rate      = DATA_TRANSFER_RATE(rsp . raw[0]);

  csd -> command_classes         = COMMAND_CLASSES(rsp . raw[1]);
  csd -> read_block_partial      = READ_BLOCK_PARTIAL(rsp . raw[1]);
  csd -> write_block_misalign    = WRITE_BLOCK_MISALIGN(rsp . raw[1]);
  csd -> read_block_misalign     = READ_BLOCK_MISALIGN(rsp . raw[1]);
  csd -> dsr_implemented         = DSR_IMPLEMENTED(rsp . raw[1]);

  csd -> read_vdd_min            = READ_VDD_MIN(rsp . raw[2]);
  csd -> read_vdd_max            = READ_VDD_MAX(rsp . raw[2]);
  csd -> write_vdd_min           = WRITE_VDD_MIN(rsp . raw[2]);
  csd -> write_vdd_max           = WRITE_VDD_MAX(rsp . raw[2]);
  csd -> size_multiplier         = SIZE_MULTIPLIER(rsp . raw[2]);
  csd -> erase_block_enable      = ERASE_BLOCK_ENABLE(rsp . raw[2]);
  csd -> erase_sector_size       = ERASE_SECTOR_SIZE(rsp . raw[2]);
  csd -> wp_group_size           = WP_GROUP_SIZE(rsp . raw[2]);

  csd -> wp_group_enable         = WP_GROUP_ENABLE(rsp_raw[3]);
  csd -> write_speed_factor      = WRITE_BLOCK_LENGTH(rsp . raw[3]);
  csd -> write_block_length      = WRITE_BLOCK_LENGTH(rsp . raw[3]);
  csd -> write_block_partial     = WRITE_BLOCK_PARTIAL(rsp . raw[3]);
  csd -> file_format_group       = FILE_FORMAT_GROUP(rsp . raw[3]);
  csd -> copy                    = COPY_FLAG(rsp . raw[3]);
  csd -> permanent_write_protect = PERM_WRITE_PROTECTION(rsp . raw[3]);
  csd -> temporary_write_protect = TEMP_WRITE_PROTECTION(rsp . raw[3]);
  csd -> file_format             = FILE_FORMAT(rsp . raw[3]);
  csd -> crc                     = CRC(rsp . raw[3]);
  csd -> always_one              = ALWAYS_ONE(rsp . raw[3]);
#endif
}

