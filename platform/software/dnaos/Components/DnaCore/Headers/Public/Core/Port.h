/****h* Core/Port
 * SUMMARY
 * Port management.
 ****
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

#ifndef DNA_CORE_PORT_PUBLIC_H
#define DNA_CORE_PORT_PUBLIC_H

#include <Core/Semaphore.h>
#include <DnaTools/DnaTools.h>

/****t* Port/port_info_t
 * SUMMARY
 * Port information type.
 *
 * SOURCE
 */

typedef struct _port_info
{
  char name[DNA_NAME_LENGTH];
  int32_t capacity;
  int32_t queue_count;
  int32_t total_count;
}
port_info_t;

/*
 ****/

extern status_t port_create (char * name, int32_t queue_length, int32_t * p_id);
extern status_t port_destroy (int32_t id);

extern status_t port_close (int32_t id);
extern status_t port_find (char * name, int32_t * p_id);

extern status_t port_poll (int32_t id, int32_t flags,
    bigtime_t timeout, int32_t * p_size);

extern status_t port_read (int32_t id, int32_t * p_code, void * buffer,
    int32_t size, int32_t flags, bigtime_t timeout);
extern status_t port_write (int32_t id, int32_t code, void * buffer,
    int32_t size, int32_t flags, bigtime_t timeout);

extern status_t port_get_info (int32_t id, port_info_t * p_info);

#endif

