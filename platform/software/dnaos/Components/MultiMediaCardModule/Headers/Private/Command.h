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

#ifndef MULTIMEDIACARD_COMMAND_PRIVATE_H
#define MULTIMEDIACARD_COMMAND_PRIVATE_H

#include <stdint.h>

#define RSPTYPE_NORSP 0UL
#define RSPTYPE_R1    (1UL << 6)
#define RSPTYPE_R2    (2UL << 6)

#define SPCDM_NOCMD   0UL
#define SPCMD_INIT    (1UL << 8)
#define SPCMD_SYNC    (2UL << 8)
#define SPCMD_ITCMD   (4UL << 8)
#define SPCMD_ITRSP   (5UL << 8)

#define PUSH_PULL     0UL
#define OPEN_DRAIN    (1UL << 11)

#define MAXLAT_5      0UL
#define MAXLAT_64     (1UL << 12)

#define TRCMD_NODATA  0UL
#define TRCMD_START   (1UL << 16)
#define TRCMD_STOP    (2UL << 16)

#define TRDIR_WRITE   0UL
#define TRDIR_READ    (1UL << 18)

#define TRTYPE_SINGLE 0UL
#define TRTYPE_MULTI  (1UL << 19)
#define TRTYPE_STREAM (2UL << 19)
#define TRTYPE_IOBYTE (4UL << 19)
#define TRTYPE_IOBCK  (5UL << 19)

#define IOCMD_NOCMD   0UL
#define IOCMD_SUSPEND (1UL << 24)
#define IOCMD_RESUME  (2UL << 24)

/*
 * Command definition.
 */

enum _mmc_class0_commands
{
  GO_IDLE_STATE = 0UL | OPEN_DRAIN,
  CMD0 = GO_IDLE_STATE,

  SEND_OP_COND = 1UL | RSPTYPE_R1 | OPEN_DRAIN | MAXLAT_64,
  CMD1 = SEND_OP_COND,

  ALL_SEND_CID = 2UL | RSPTYPE_R2 | MAXLAT_64 | OPEN_DRAIN,
  CMD2 = ALL_SEND_CID,

  SEND_RELATIVE_ADDR = 3UL | RSPTYPE_R1 | MAXLAT_64,
  CMD3 = SEND_RELATIVE_ADDR,

  SELECT_DESELECT_CARD = 7UL | RSPTYPE_R1 | MAXLAT_64,
  CMD7 = SELECT_DESELECT_CARD,

  SEND_IF_COND = 8UL | RSPTYPE_R1 | OPEN_DRAIN | MAXLAT_64,
  CMD8 = SEND_IF_COND,

  SEND_CSD = 9UL | RSPTYPE_R2 | MAXLAT_64,
  CMD9 = SEND_CSD,

  SEND_CID = 10UL | RSPTYPE_R2 | MAXLAT_64,
  CMD10 = SEND_CID,

  STOP_TRANSMISSION = 12UL | RSPTYPE_R1 | MAXLAT_64 | TRCMD_STOP,
  CMD12 = STOP_TRANSMISSION,

  SEND_STATUS = 13UL | RSPTYPE_R1 | MAXLAT_64,
  CMD13 = SEND_STATUS,

  SET_BLOCK_LENGTH = 16UL | RSPTYPE_R1 | MAXLAT_64,
  CMD16 = SET_BLOCK_LENGTH,

  READ_SINGLE_BLOCK = 17UL | RSPTYPE_R1 | MAXLAT_64 | TRDIR_READ | TRCMD_START,
  CMD17 = READ_SINGLE_BLOCK,

  WRITE_SINGLE_BLOCK = 24UL | RSPTYPE_R1 | MAXLAT_64 | TRCMD_START,
  CMD24 = WRITE_SINGLE_BLOCK,
}
mmc_class0_commands;

enum _mmc_class8_commands
{
  APP_CMD = 55 | RSPTYPE_R1 | MAXLAT_64,
  CMD55 = APP_CMD,
}
mmc_class8_commands;

enum _mmc_sd_commands
{
  SD_SET_BUS_WIDTH = 6UL | RSPTYPE_R1 | MAXLAT_64,
  ACMD6 = SD_SET_BUS_WIDTH,

  SD_SEND_STATUS = 13UL | RSPTYPE_R1 | MAXLAT_64,
  ACMD13 = SD_SEND_STATUS,

  SD_CMD_APP_SEND_OP_COND = 41UL | RSPTYPE_R1 | MAXLAT_64,
  ACMD41 = SD_CMD_APP_SEND_OP_COND,

  SD_CMD_READ_OCR = 58UL | RSPTYPE_R1 | MAXLAT_64,
  ACMD51 = SD_CMD_READ_OCR
}
mmc_sd_commands;

#endif

