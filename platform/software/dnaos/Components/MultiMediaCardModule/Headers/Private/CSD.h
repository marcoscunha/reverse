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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the 
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program.If not, see <http://www.gnu.org/licenses/>. 
 */

#ifndef MULTIMEDIACARD_CSD_PRIVATE_H
#define MULTIMEDIACARD_CSD_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>

#define CSD_STRUCTURE(x)         (x & (0xC0UL  << 24)) >> 30
#define READ_ACCESS_TIME(x)      (x & (0xFFUL  << 16)) >> 16
#define READ_ACCESS_CYCLES(x)    (x & (0xFFUL  <<  8)) >>  8
#define DATA_TRANSFER_RATE(x)    (x & (0xFFUL  <<  0)) >>  0

#define COMMAND_CLASSES(x)       (x & (0xFFFUL << 20)) >> 20
#define READ_BLOCK_LENGTH(x)     (x & (0x0FUL  << 16)) >> 16
#define READ_BLOCK_PARTIAL(x)    (x & (0x01UL  << 15)) >> 15
#define WRITE_BLOCK_MISALIGN(x)  (x & (0x01UL  << 14)) >> 14
#define READ_BLOCK_MISALIGN(x)   (x & (0x01UL  << 13)) >> 13
#define DSR_IMPLEMENTED(x)       (x & (0x01UL  << 12)) >> 12

#define DEVICE_SIZE_V1(x,y)      (x & (0x3FFUL <<  0)) <<  2 | \
                                 (y & (0xC0UL  << 24)) >> 30

#define DEVICE_SIZE_V2(x,y)      (x & (0x3FUL  <<  0)) << 16 | \
                                 (y & (0xFFFFUL<< 16)) >> 16

#define READ_VDD_MIN(x)          (x & (0x38UL  << 24)) >> 27
#define READ_VDD_MAX(x)          (x & (0x07UL  << 24)) >> 24
#define WRITE_VDD_MIN(x)         (x & (0xE0UL  << 16)) >> 21
#define WRITE_VDD_MAX(x)         (x & (0x1CUL  << 16)) >> 18
#define SIZE_MULTIPLIER(x)       (x & (0x38UL  << 12)) >> 15
#define ERASE_BLOCK_ENABLE(x)    (x & (0x01UL  << 14)) >> 14
#define ERASE_SECTOR_SIZE(x)     (x & (0x3F8UL <<  4)) >>  7
#define WP_GROUP_SIZE(x)         (x & (0x07UL  <<  0)) >>  0

#define WP_GROUP_ENABLE(x)       (x & (0x80UL  << 24)) >> 31
#define WRITE_SPEED_FACTOR(x)    (x & (0x1CUL  << 24)) >> 26
#define WRITE_BLOCK_LENGTH(x)    (x & (0x3CUL  << 20)) >> 22
#define WRITE_BLOCK_PARTIAL(x)   (x & (0x01UL  << 21)) >> 21
#define FILE_FORMAT_GROUP(x)     (x & (0x01UL  << 15)) >> 15
#define COPY_FLAG(x)             (x & (0x01UL  << 14)) >> 14
#define PERM_WRITE_PROTECTION(x) (x & (0x01UL  << 13)) >> 13
#define TEMP_WRITE_PROTECTION(x) (x & (0x01UL  << 12)) >> 12
#define FILE_FORMAT(x)           (x & (0x0CUL  <<  8)) >> 10
#define CRC(x)                   (x & (0xFEUL  <<  0)) >>  0
#define ALWAYS_ONE(x)            (x & (0x01UL  <<  0)) >>  0

typedef enum _mmc_csd_structure
{
  CSD_V1_STRUCTURE = 0,
  CSD_V2_STRUCTURE = 1
}
mmc_csd_structure_t;

typedef struct _mmc_csd
{
  mmc_csd_structure_t csd_structure;

  uint8_t read_access_time;
  uint8_t read_access_cycles;
  uint8_t data_transfer_rate;
  uint16_t command_classes;

  uint32_t device_size;
  uint8_t size_multiplier;
  uint8_t file_format;

  uint8_t read_block_length;
  bool read_block_partial;
  bool read_block_misalign;

  uint8_t write_block_length;
  uint8_t write_speed_factor;
  bool write_block_partial;
  bool write_block_misalign;

  uint8_t read_vdd_min;
  uint8_t read_vdd_max;
  uint8_t write_vdd_min;
  uint8_t write_vdd_max;

  bool erase_block_enable;
  uint8_t erase_sector_size;

  bool wp_group_enable;
  uint8_t wp_group_size;

  bool dsr_implemented;
  bool file_format_group;
  bool copy;
  bool permanent_write_protect;
  bool temporary_write_protect;

  uint8_t crc;
  bool always_one;
}                                  
mmc_csd_t;                         
                                   
extern void mmc_build_csd (uint32_t response[4], mmc_csd_t * csd);
extern void mmc_show_csd (mmc_csd_t csd);

#endif                             
