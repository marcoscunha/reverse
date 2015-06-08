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

#ifndef MULTIMEDIACARD_OPERATION_PRIVATE_H
#define MULTIMEDIACARD_OPERATION_PRIVATE_H

#include <stdint.h>
#include <MultiMediaCard/Card.h>
#include <MultiMediaCard/Operation.h>
#include <DnaTools/DnaTools.h>

typedef enum _mmc_state_t
{
  MMC_IDLE_STATE,
  MMC_SDHC_DISCOVER,
  MMC_SD_DISCOVER,
  MMC_GET_IDENTIFIER,
  MMC_GET_ADDRESS,
  MMC_GET_INFORMATION,
  MMC_CONFIGURE_TRANSFER,
  MMC_EXECUTE_SINGLE,
  MMC_EXECUTE_MULTIPLE,
  MMC_END_STATE
}
mmc_state_t;

extern status_t mmc_card_create (mmc_card_t * card, mmc_callbacks_t callbacks);
extern status_t mmc_card_destroy (mmc_card_t card);
extern status_t mmc_execute (mmc_card_t card, mmc_operation_t operation,
    void * buffer, int64_t block_start, int32_t block_count);

#endif
