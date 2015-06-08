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

#ifndef MULTIMEDIACARD_CARD_PRIVATE_H
#define MULTIMEDIACARD_CARD_PRIVATE_H

#include <stdint.h>
#include <Private/CID.h>
#include <Private/CSD.h>
#include <Private/RCA.h>
#include <MultiMediaCard/Card.h>
#include <DnaTools/DnaTools.h>

enum mmc_card_state
{
  MMC_CARD_IDLE = 0,
  MMC_CARD_READY,
  MMC_CARD_IDENT,
  MMC_CARD_STDBY,
  MMC_CARD_TRAN,
  MMC_CARD_DATA,
  MMC_CARD_RCV,
  MMC_CARD_PRG,
  MMC_CARD_DIS
};

typedef union _mmc_card_status
{
  uint32_t raw;

  struct _mmc_card_status_bits
  {
    uint32_t test_mode            :2;
    uint32_t app_specific_command :2;
    uint32_t reserved_0           :1;
    uint32_t application_command  :1;
    uint32_t reserved_1           :1;
    uint32_t switch_error         :1;
    uint32_t ready_for_data       :1;
    uint32_t current_state        :4;
    uint32_t erase_reset          :1;
    uint32_t reserved_2           :1;
    uint32_t wp_erase_skip        :1;
    uint32_t cid_csd_overwrite    :1;
    uint32_t overrun              :1;
    uint32_t underrun             :1;
    uint32_t error                :1;
    uint32_t cc_error             :1;
    uint32_t card_ecc_failed      :1;
    uint32_t illegal_command      :1;
    uint32_t com_crc_error        :1;
    uint32_t lock_unlock_failed   :1;
    uint32_t card_is_locked       :1;
    uint32_t wp_violation         :1;
    uint32_t erase_param          :1;
    uint32_t erase_seq_error      :1;
    uint32_t block_len_error      :1;
    uint32_t address_misaligned   :1;
    uint32_t address_out_of_range :1;
  }
  bits;
}
mmc_card_status_t;

typedef struct _mmc_card_private
{
  struct _mmc_card card;
  mmc_callbacks_t callbacks;

  mmc_cid_t cid;
  mmc_csd_t csd;
  mmc_rca_t rca;
}
* mmc_card_private_t;

extern void mmc_show_card_status (mmc_card_status_t status);
  
#endif
