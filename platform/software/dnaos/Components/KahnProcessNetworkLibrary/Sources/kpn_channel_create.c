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

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <DnaTools/DnaTools.h>
#include <KahnProcessNetwork/KahnProcessNetwork.h>

kpn_status_t kpn_channel_create (char * name,
    int32_t buffer_size, kpn_channel_t * channel)
{
  kpn_channel_t c = NULL;
  
  watch (kpn_status_t)
  {
    c = malloc (sizeof (struct _kpn_channel));
    ensure (c != NULL, KPN_OUT_OF_MEM);

    c -> fd = open (name, O_RDWR);
    check (vfs_error, c -> fd != -1, KPN_NO_ENTRY);

    if (buffer_size != 0)
    {
      c -> data = malloc (buffer_size);
      check (data_error, c -> data != NULL, KPN_OUT_OF_MEM);

      c -> size = buffer_size;
    }
    else
    {
      c -> data = NULL;
      c -> size = 0;
    }

    c -> name = malloc (strlen (name) + 1);
    check (name_error, c -> name != NULL, KPN_OUT_OF_MEM);

    c -> level = 0;
    c -> position = 0;
    strcpy (c -> name, name);

    *channel = c;
    DCACHE_FLUSH(channel, sizeof(kpn_channel_t));
    return KPN_OK;
  }

  rescue (name_error)
  {
    if (buffer_size != 0) free (c -> data);
  }

  rescue (data_error)
  {
    close (c -> fd);
  }

  rescue (vfs_error)
  {
    free (c);
    leave;
  }
}

