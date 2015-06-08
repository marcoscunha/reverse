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
#include <DnaTools/DnaTools.h>

static char * status_string[] = 
{
  "application command", "ready for data",
  "erase reset", "card ecc disabled",
  "wp erase skip", "csd overwrite",
  "error", "cc_error",
  "card_ecc_failed", "illegal command",
  "com CRC error", "lock/unlock failed",
  "card is locked", "wp violation",
  "erase param error", "erase seq error",
  "block_len_error", "address error",
  "out of range", NULL
};

static uint32_t status_code[] =
{
  (1UL << 5), (1UL << 8),
  (1UL << 13), (1UL << 14),
  (1UL << 15), (1UL << 16),
  (1UL << 19), (1UL << 20),
  (1UL << 21), (1UL << 22),
  (1UL << 23), (1UL << 24),
  (1UL << 25), (1UL << 26),
  (1UL << 27), (1UL << 28),
  (1UL << 29), (1UL << 30),
  (1UL << 31)
};

static char * state_string[] = 
{
  "idle", "ready", "ident", "stdby", "tran",
  "data", "rcv", "prog", "dis", "rsv", "rsv",
  "rsv", "rsv", "rsv", "rsv", "rsvio"
};

void mmc_show_card_status (mmc_card_status_t status)
{
  dna_printf ("Card state = %s\r\n",
      state_string[status . bits . current_state]);

  for (int32_t index = 0; status_string[index] != NULL; index += 1)
  {
    if ((status . raw & status_code[index]) != 0)
    {
      dna_printf ("%s\r\n", status_string[index]);
    }
  }

}

