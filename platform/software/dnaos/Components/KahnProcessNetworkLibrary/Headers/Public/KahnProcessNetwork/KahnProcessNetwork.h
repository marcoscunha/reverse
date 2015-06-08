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

#ifndef _DPN_H_
#define _DPN_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum _kpn_status
{
  KPN_OK,
  KPN_ERROR,
  KPN_OUT_OF_MEM,
  KPN_NO_ENTRY
}
kpn_status_t;

typedef struct _kpn_channel
{
  char * name;
  int16_t fd;
  int32_t size;
  int32_t level;
  int32_t position;
  uint8_t * data;
}
* kpn_channel_t;

kpn_status_t kpn_channel_create (char * name,
    int32_t buffer_size, kpn_channel_t * channel); 

extern kpn_status_t kpn_channel_read (kpn_channel_t c,
    void * data, int32_t size);
extern kpn_status_t kpn_channel_write (kpn_channel_t c,
    void * data, int32_t size);

extern kpn_status_t kpn_channel_skip (kpn_channel_t c, int32_t size);
extern kpn_status_t kpn_channel_purge (kpn_channel_t c, bool flush);

#endif

