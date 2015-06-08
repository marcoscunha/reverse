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

#include <Private/RCA.h>
#include <DnaTools/DnaTools.h>

static char * rca_string[] = 
{
  "application command", "ready for data",
  "error", "illegal command",
  "com CRC error", NULL
};

static uint32_t rca_code[] =
{
  (1UL << 5), (1UL << 8),
  (1UL << 19), (1UL << 22),
  (1UL << 23),
};

static char * state_string[] = 
{
  "idle", "ready", "ident", "stdby", "tran",
  "data", "rcv", "prog", "dis", "rsv", "rsv",
  "rsv", "rsv", "rsv", "rsv", "rsvio"
};

void mmc_show_rca (mmc_rca_t rca)
{
  dna_printf ("Raw RCA = 0x%x\r\n", rca . raw);
  dna_printf ("Card state = %s\r\n", state_string[rca . bits . current_state]);
  dna_printf ("RCA = 0x%x\r\n", rca . bits . rca);

  for (int32_t index = 0; rca_string[index] != NULL; index += 1)
  {
    if ((rca . raw & rca_code[index]) != 0)
    {
      dna_printf ("%s\r\n", rca_string[index]);
    }
  }
}

