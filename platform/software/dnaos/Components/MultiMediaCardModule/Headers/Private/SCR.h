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

#ifndef MULTIMEDIACARD_SCR_PRIVATE_H
#define MULTIMEDIACARD_SCR_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>

#define SCR_STRUCTURE(x)           (x & (0xFFUL << 24)) >> 28
#define SD_SPECIFICATION(x)        (x & (0x0FUL << 24)) >> 24
#define DATA_STATUS_AFTER_ERASE(x) (x & (0x80UL << 16)) >> 23
#define SD_SECURITY(x)             (x & (0x70UL << 16)) >> 20
#define SD_BUS_WIDTH(x)            (x & (0x0FUL << 16)) >> 16

typedef enum _mmc_scr_structure
{
  SCR_V1_STRUCTURE = 0
}
mmc_scr_structure_t;

typedef enum _mmc_scr_specification
{
  SCR_V10_101_SPEC = 0,
  SCR_V110_SPEC = 1,
  SCR_V200_SPEC = 2
}
mmc_scr_specification_t;

typedef enum _mmc_scr_security
{
  SCR_NO_SECURITY = 0,
  SCR_V101_SECURITY = 2,
  SCR_V200_SECURITY = 3
}
mmc_scr_security_t;

typedef enum _mmc_scr_bus_width
{
  SCR_1BIT_BUS_WIDTH = 0,
  SCR_4BIT_BUS_WIDTH = 2
}
mmc_scr_bus_width_t;

typedef struct _mmc_scr
{
  mmc_scr_structure_t scr_structure;
  mmc_scr_specification_t sd_specification;
  bool data_status_after_erase;
  mmc_scr_security_t security_support;
  mmc_scr_bus_width_t data_bus_width;
}                                  
mmc_scr_t;                         
                                   
extern void mmc_build_scr (uint32_t response[4], mmc_scr_t * scr);
extern void mmc_show_scr (mmc_scr_t scr);

#endif                             
