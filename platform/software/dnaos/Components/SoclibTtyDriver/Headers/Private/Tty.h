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

#ifndef SOCLIB_TTY_H
#define SOCLIB_TTY_H

#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

typedef struct soclib_tty_port {
  uint32_t write;
  uint32_t status;
  uint32_t read;
} * soclib_tty_port_t;

typedef struct soclib_tty_config {
  uint32_t irq;
  soclib_tty_port_t port;
} soclib_tty_config_t;

typedef struct soclib_tty {
  char * name;

  struct _buffer
  {
    bool empty;
    char data;
  }
  buffer;

  uint32_t irq;
  soclib_tty_port_t port;
  int32_t sem_id;
} soclib_tty_t;

extern uint32_t SOCLIB_TTY_NDEV;
extern soclib_tty_config_t SOCLIB_TTY_DEVICES[];

extern soclib_tty_t * TTY;

extern int32_t soclib_tty_isr (int32_t itn);

extern status_t soclib_tty_open (char * name, int32_t mode, void ** data);
extern status_t soclib_tty_close (void * data);
extern status_t soclib_tty_free (void * data);

extern status_t soclib_tty_read (void * handler, void * destination, int64_t offset, int32_t * p_count);
extern status_t soclib_tty_write (void * handler, void * source, int64_t offset, int32_t * p_count);
extern status_t soclib_tty_control (void * handler, int32_t operation, void * data, int32_t * p_res);

#endif

