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

#include <Private/Card.h>
#include <Private/Command.h>
#include <Private/Operation.h>
#include <DnaTools/DnaTools.h>

status_t mmc_execute (mmc_card_t a_card, mmc_operation_t operation,
    void * buffer, int64_t block_start, int32_t block_count)
{
  status_t status;
  bool loop = true;
  uint32_t response[4];
  int32_t block_offset = 0, multiplier = 0, word_count;
  mmc_state_t state = MMC_CONFIGURE_TRANSFER;
  mmc_card_private_t card = (mmc_card_private_t)a_card;
  mmc_card_status_t card_status;

  watch (status_t)
  {
    while (loop) switch (state)
    {
      case MMC_CONFIGURE_TRANSFER :
        {
          dna_log(VERBOSE_LEVEL, "[MMC_CONFIGURE_TRANSFER]");

          status = card -> callbacks . send_command
            (SELECT_DESELECT_CARD, card -> rca . raw, response);
          ensure (status == DNA_OK, status);

          status = card -> callbacks . send_command
            (APP_CMD, card -> rca . raw, response);
          check (error, status == DNA_OK, status);

          status = card -> callbacks . send_command
            (SD_SET_BUS_WIDTH, 2, response);
          check (error, status == DNA_OK, status);

          status = card -> callbacks . send_command
            (SET_BLOCK_LENGTH, 512, response);
          check (error, status == DNA_OK, status);

          state = block_count == 1 ? MMC_EXECUTE_SINGLE : MMC_EXECUTE_MULTIPLE;
          break;
        }

      case MMC_EXECUTE_SINGLE :
        {
          dna_log(VERBOSE_LEVEL, "[MMC_EXECUTE_SINGLE]");

          multiplier = (card -> card . type == MMC_SD_CARD) ? 9 : 0;
          block_offset = block_start << multiplier;

          switch (operation)
          {
            case MMC_READ :
              {
                status = card -> callbacks . send_command
                  (READ_SINGLE_BLOCK, block_offset, response);
                check (error, status == DNA_OK, status);

                word_count = (block_count * 512) >> 2;
                status = card -> callbacks . read (buffer, word_count);
                check (error, status == DNA_OK, status);

                break;
              }

            case MMC_WRITE :
              {
                status = card -> callbacks . send_command
                  (WRITE_SINGLE_BLOCK, block_offset, response);
                check (error, status == DNA_OK, status);

                word_count = (block_count * 512) >> 2;
                status = card -> callbacks . write (buffer, word_count);
                check (error, status == DNA_OK, status);

                do
                {
                  status = card -> callbacks . send_command
                    (SEND_STATUS, card -> rca . raw, response);
                  check (error, status == DNA_OK, status);
                  card_status . raw = response[0];
                }
                while (card_status . bits . ready_for_data == 0);

                break;
              }
          }

          state = MMC_END_STATE;
          break;
        }

      case MMC_END_STATE :
        {
          card -> callbacks . send_command (SELECT_DESELECT_CARD, 0, response);
          loop = false;

          break;
        }

      default :
        {
          dna_log(PANIC_LEVEL, "Invalid MMC state.");
          return DNA_ERROR;
        }
    }

    return DNA_OK;
  }

  rescue (error)
  {
    card_status . raw = response[0];
    leave;
  }
}

