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

#include <stdbool.h>
#include <Private/MultiMediaCard.h>
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

status_t mmc_card_create (mmc_card_t * p_card, mmc_callbacks_t callbacks)
{
  status_t status = DNA_OK;
  bool is_sdhc = is_sdhc, has_ccs = false, loop = true;
  uint32_t block_size = 0, block_count = 0, multiplier;
  uint32_t response[4];
  mmc_state_t state = MMC_IDLE_STATE;
  mmc_card_private_t card = NULL;
  mmc_ocr_t ocr = { .raw = 0 };

  watch (status_t)
  {
    ensure (p_card != NULL, DNA_BAD_ARGUMENT);

    /*
     * Allocate the p_card object.
     */

    card = kernel_malloc (sizeof (struct _mmc_card_private), true);
    card -> callbacks = callbacks;

    /*
     * Execute the initialization state machine.
     */

    while (loop) switch (state)
    {
      case MMC_IDLE_STATE :
        {
          dna_log(VERBOSE_LEVEL, "[MMC_IDLE_STATE]");
          status = callbacks . send_command (GO_IDLE_STATE, 0, response);
          check (error, status == DNA_OK, status);

          state = is_sdhc ? MMC_SDHC_DISCOVER : MMC_SD_DISCOVER;
          break;
        }

      case MMC_SDHC_DISCOVER :
        {
          dna_log(VERBOSE_LEVEL, "[MMC_SDHC_DISCOVER]");
          status = callbacks . send_command (SEND_IF_COND, 0x1AA, response);

          is_sdhc = (response[0] == 0x1AA) ? true : false;
          state = is_sdhc ? MMC_SD_DISCOVER : MMC_IDLE_STATE;

          break;
        }

      case MMC_SD_DISCOVER :
        {
          dna_log(VERBOSE_LEVEL, "[MMC_SD_DISCOVER]");

          status = callbacks . send_command (APP_CMD, 0, response);
          check (error, status == DNA_OK, status);

          status = callbacks . send_command (SD_CMD_APP_SEND_OP_COND,
              MMC_VOLTAGE_RANGE | (is_sdhc ? MMC_SD_HCS : 0), response);
          check (error, status == DNA_OK, status);

          ocr . raw = response[0];

          if (ocr . bits . not_busy == 1)
          {
            has_ccs = (ocr . bits . capacity_status == 1);
            card -> card . type = is_sdhc && has_ccs ?
              MMC_SDHC_CARD : MMC_SD_CARD;
            state = MMC_GET_IDENTIFIER;
          }

          break;
        }

      case MMC_GET_IDENTIFIER :
        {
          dna_log(VERBOSE_LEVEL, "[MMC_GET_IDENTIFIER]");

          status = callbacks . send_command (ALL_SEND_CID, 0, response);
          check (error, status == DNA_OK, status);

          mmc_build_cid (response, & card -> cid);
          state = MMC_GET_ADDRESS;

          break;
        }

      case MMC_GET_ADDRESS :
        {
          dna_log(VERBOSE_LEVEL, "[MMC_GET_ADDRESS]");

          status = callbacks . send_command (SEND_RELATIVE_ADDR, 0, response);
          check (error, status == DNA_OK, status);

          card -> rca . raw = response[0];
          state = MMC_GET_INFORMATION;

          break;
        }

      case MMC_GET_INFORMATION :
        {
          dna_log(VERBOSE_LEVEL, "[MMC_GET_INFORMATION]");

          status = callbacks . send_command (SEND_CSD,
              card -> rca . raw, response);
          check (error, status == DNA_OK, status);

          mmc_build_csd (response, & card -> csd);

          switch (card -> csd . csd_structure)
          {
            case CSD_V1_STRUCTURE :
              multiplier = 1 << (card -> csd . size_multiplier + 2); 
              block_size = 1 << card -> csd . read_block_length;
              block_count = (card -> csd . device_size + 1) * multiplier;
              break;

            case CSD_V2_STRUCTURE :
              block_size = 512;
              block_count = card -> csd . device_size + 1;
              break;
          }

          card -> card . info . block_size = block_size;
          card -> card . info . block_count = block_count;

          state = MMC_END_STATE;
          break;
        }

      case MMC_END_STATE :
        {
          loop = false;
          break;
        }

      default :
        {
          dna_log(PANIC_LEVEL, "Invalid MMC state.");
          return DNA_ERROR;
        }
    }

    /*
     * Fill-in some remaining parameters.
     */

    card -> card . info . removable = true;
    card -> card . info . read_only = true;
    card -> card . info . write_once = false;

    *p_card = & card -> card;
    return DNA_OK;
  }

  rescue (error)
  {
    kernel_free (card);
    leave;
  }
}
