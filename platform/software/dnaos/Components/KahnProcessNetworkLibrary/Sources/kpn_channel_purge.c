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
#include <stdbool.h>
#include <unistd.h>

#include <DnaTools/DnaTools.h>
#include <KahnProcessNetwork/KahnProcessNetwork.h>

kpn_status_t kpn_channel_purge (kpn_channel_t c, bool flush)
{
  int32_t res = 0;

  watch (kpn_status_t)
  {
    ensure (c != NULL, KPN_ERROR);

    if (flush)
    {
      res = write (c -> fd, c -> data, c -> size);
      ensure (res != -1, KPN_ERROR);
    }

    c -> level = 0;
    c -> position = 0;

    return KPN_OK;
  }
}
