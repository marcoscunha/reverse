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

#ifndef PC_PLATFORM_TTY_PRIVATE_H
#define PC_PLATFORM_TTY_PRIVATE_H

#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

typedef struct pc_tty_port
{
  uint32_t write;
  uint32_t status;
  uint32_t read;
}
* pc_tty_port_t;

typedef struct pc_tty_config
{
  uint32_t irq;
  pc_tty_port_t port;
} pc_tty_config_t;

typedef struct pc_tty
{
  char * name;

  struct _buffer
  {
    bool empty;
    char data;
  }
  buffer;

  uint32_t irq;
  pc_tty_port_t port;
  int32_t sem_id;
} pc_tty_t;

extern uint32_t PC_TTY_NDEV;
extern pc_tty_config_t PC_TTY_DEVICES[];

extern pc_tty_t * TTY;
extern device_cmd_t pc_tty_commands;

extern int32_t pc_tty_isr (void * data);

extern status_t pc_tty_open (char * name, int32_t mode, void ** data);
extern status_t pc_tty_close (void * data);
extern status_t pc_tty_free (void * data);

extern status_t pc_tty_read (void * handler, void * destination,
    int64_t offset, int32_t * p_count);
extern status_t pc_tty_write (void * handler, void * source,
    int64_t offset, int32_t * p_count);

extern status_t pc_tty_control (void * handler, int32_t function,
    va_list arguments, int32_t * p_ret);

#endif

