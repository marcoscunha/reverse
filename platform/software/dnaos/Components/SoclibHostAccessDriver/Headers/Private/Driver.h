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

#ifndef SOCLIB_FDACCESS_PRIVATE_H
#define SOCLIB_FDACCESS_PRIVATE_H

#include <SoclibHostAccessDriver/Driver.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

typedef struct _fdaccess_port
{
  uint32_t FD_ACCESS_FD;
  uint32_t FD_ACCESS_BUFFER;
  uint32_t FD_ACCESS_SIZE;
  uint32_t FD_ACCESS_HOW;
  uint32_t FD_ACCESS_MODE;
  uint32_t FD_ACCESS_OP;
  uint32_t FD_ACCESS_RETVAL;
  uint32_t FD_ACCESS_ERRNO;
  uint32_t FD_ACCESS_IRQ_ENABLE;
} * fdaccess_port_t;

typedef struct _fdaccess_config
{
  uint32_t entries;
  uint32_t itn;
  fdaccess_port_t port;
}
fdaccess_config_t;

typedef struct _fdaccess
{
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare[7];
  int32_t descriptor;
  int32_t errno;
  fdaccess_port_t port;
}
* fdaccess_t;

enum fdaccess_commands
{
  FD_ACCESS_NOOP,
  FD_ACCESS_OPEN, 
  FD_ACCESS_CLOSE, 
  FD_ACCESS_READ,
  FD_ACCESS_WRITE,
  FD_ACCESS_LSEEK
}; 

/*
 * Definitions
 */

extern uint32_t SOCLIB_FDACCESS_NDEV;
extern fdaccess_config_t SOCLIB_FDACCESS_DEVICES[];

extern device_cmd_t fdaccess_commands;
char ** fdaccess_devices;

/*
 * Methods
 */

extern status_t fdaccess_init_hardware (void);
extern status_t fdaccess_init_driver (void);
extern void fdaccess_uninit_driver (void);
extern const char ** fdaccess_publish_devices (void);
extern device_cmd_t * fdaccess_find_device (const char * name);

extern status_t fdaccess_open (char * name, int32_t mode, void ** data);
extern status_t fdaccess_close (void * data);
extern status_t fdaccess_free (void * data);

extern status_t fdaccess_read (void * handler, void * destination,
    int64_t offset, int32_t * p_count);
extern status_t fdaccess_write (void * handler, void * source,
    int64_t offset, int32_t * p_count);
extern status_t fdaccess_control (void * handler, int32_t operation,
    va_list arguments, int32_t * p_res);

#endif

