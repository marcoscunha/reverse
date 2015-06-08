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

#ifndef RDV_CHANNEL_PRIVATE_H
#define RDV_CHANNEL_PRIVATE_H

#include <RendezVousChannelDriver/Driver.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

typedef struct channel_rdv
{
  int32_t sem __attribute__((aligned(32)));
  int32_t spare1[7];
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare2[7];

  struct {
    const void * buffer;
    volatile bool ready;
  } setter;

  struct {
    const void * buffer;
    volatile bool ready;
  } getter;
}
channel_rdv_t;

extern int32_t CHANNEL_RDV_NDEV;

extern const char ** rdv_devices;
extern channel_rdv_t * rdv;

extern status_t rdv_init_hardware (void);
extern status_t rdv_init_driver (void);
extern void rdv_uninit_driver (void);
extern const char ** rdv_publish_devices (void);
extern device_cmd_t * rdv_find_device (const char * name);

extern status_t rdv_open (char * name, int32_t mode, void ** data);
extern status_t rdv_close (void * data);
extern status_t rdv_free (void * data);

extern status_t rdv_read (void * handler, void * destination,
    int64_t offset, int32_t * p_count);
extern status_t rdv_write (void * handler, void * source,
    int64_t offset, int32_t * p_count);
extern status_t rdv_control (void * handler, int32_t operation,
    va_list arguments, int32_t * p_res);

#endif

