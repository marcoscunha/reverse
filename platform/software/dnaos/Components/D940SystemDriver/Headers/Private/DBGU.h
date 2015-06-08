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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the 
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program.If not, see <http://www.gnu.org/licenses/>. 
 */

#ifndef D940_DBGU_H
#define D940_DBGU_H

#include <stdint.h>
#include <stdarg.h>
#include <Core/Core.h>
#include <Platform/Platform.h>

/*
 * Definition of DBGU driver.
 */

typedef struct _d940_dbgu_driver
{
  d940_dbgu_t port;
  int32_t semaphore;

  int32_t buffer_index;
  int32_t buffer_length;
  char input_buffer[4096];
}
d940_dbgu_driver_t;

extern d940_dbgu_driver_t d940_dbgu_driver;

/*
 * Driver functions.
 */

extern status_t d940_dbgu_open (char * name, int32_t mode, void ** data);
extern status_t d940_dbgu_close (void * data);
extern status_t d940_dbgu_free (void * data);

extern status_t d940_dbgu_read (void * handler, void * destination,
    int64_t offset, int32_t * p_count);
extern status_t d940_dbgu_write (void * handler, void * source,
    int64_t offset, int32_t * p_count);
extern status_t d940_dbgu_control (void * handler, int32_t operation,
    va_list data, int32_t * p_res);

extern int32_t d940_dbgu_isr (void * data);

#endif

