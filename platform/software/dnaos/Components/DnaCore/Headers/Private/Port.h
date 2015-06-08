/****h* CorePrivate/PortPrivate
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

#ifndef DNA_CORE_PORT_PRIVATE_H
#define DNA_CORE_PORT_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>

#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

/****t* PortPrivate/port_id_t
 * SUMMARY
 * Port ID type.
 *
 * SOURCE
 */

typedef union _port_id
{
  int32_t raw;

  struct _port_id_s
  {
    uint16_t value;
    uint16_t index;
  }
  s;
}
port_id_t;

/*
 ****/

/****t* PortPrivate/message_t
 * SUMMARY
 * Message type.
 *
 * SOURCE
 */

typedef struct _message
{
  queue_link_t link;
  int32_t code;
  int32_t size;
  int8_t * buffer;
}
* message_t;

/*
 ****/

/****t* PortPrivate/port_t
 * SUMMARY
 * Port type.
 *
 * SOURCE
 */

typedef struct _port
{
  // XXX: Do not change the position of first element!
  queue_link_t link __attribute__((aligned(32)));
  port_id_t id;

  #ifdef WRITEBACK
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare[7];
  #else
  spinlock_t lock;
  #endif

  bool closed;
  port_info_t info;

  int32_t read_sem;
  int32_t write_sem;

  struct _message * data;
  queue_t message;

  queue_t mailbox;
}
* port_t;

/*
 ****/

/****t* PortPrivate/port_pool_t
 * SUMMARY
 * Port manager type.
 *
 * SOURCE
 */

typedef struct _port_pool
{
  #ifdef WRITEBACK
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare[7];
  #else
  spinlock_t lock;
  #endif
  int16_t counter;
  struct _port data[DNA_MAX_PORT];
  queue_t port;
}
port_pool_t;

/*
 ****/

extern port_pool_t port_pool;

#endif

