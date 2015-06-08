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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <DnaTools/DnaTools.h>
#include <KahnProcessNetwork/KahnProcessNetwork.h>

kpn_status_t kpn_channel_skip (kpn_channel_t c, int32_t size)
{
  int32_t counter = size, res = 0;
  int32_t bytes_to_transfer = 0;
  uint8_t * waste;

  watch (kpn_status_t)
  {
    if (c -> data == NULL)
    {
      waste = malloc (size);
      ensure (waste != NULL, KPN_OUT_OF_MEM);

      res = read (c -> fd, waste, size);
      free (waste);

      ensure (res != -1, KPN_ERROR);
    }
    else
    {
      while (counter != 0)
      {
        if (c -> level == 0)
        {
          res = read (c -> fd, c -> data, c -> size);
          c -> level = c -> size;
          c -> position = 0;
        }

        bytes_to_transfer = (counter <= c -> level) ?
          counter : c -> level;

        c -> position += bytes_to_transfer;
        c -> level -= bytes_to_transfer;
        counter -= bytes_to_transfer;
      }
    }

    return KPN_OK;
  }
}
