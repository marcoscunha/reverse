/****h* CorePrivate/ThreadPrivate
 * SUMMARY
 * Thread management.
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

#ifndef DNA_CORE_THREAD_PRIVATE_H
#define DNA_CORE_THREAD_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include <Private/Alarm.h>
#include <Private/Semaphore.h>
#include <Core/Core.h>

#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****t* ThreadPrivate/thread_id_t
 * SUMMARY
 * Thread ID type.
 *
 * SOURCE
 */

typedef union _thread_id
{
  int32_t raw;

  struct _thread_id_s
  {
    uint16_t value;
    uint8_t group;
    uint8_t index;
  }
  s;
}
thread_id_t;

/*
 ****/

/****t* ThreadPrivate/thread_signature_t
 * SUMMARY
 * Thread signature.
 */

typedef struct _thread_signature
{
  thread_handler_t handler;
  interrupt_status_t it_restore;
  void * arguments;
  int32_t return_value;
}
thread_signature_t;

/*
 ****/

/****t* ThreadPrivate/thread_t
 * SUMMARY
 * Thread type.
 *
 * SOURCE
 */

typedef struct _thread
{
  queue_link_t link __attribute__((aligned(32)));
;
  queue_t wait;
  #ifdef WRITEBACK // Does not change this placement
  spinlock_t lock __attribute__((aligned(32)));
  uint32_t spare1[7];
  #else
  spinlock_t lock;
  #endif
  queue_t * resource_queue;
  thread_id_t id;
  thread_info_t info;

  bool stack_allocated;
  thread_signature_t signature;
  cpu_context_t context;
}
* thread_t;

/*
 ****/

/****t* ThreadPrivate/thread_pool_t
 * SUMMARY
 * Thread pool type.
 *
 * SOURCE
 */

typedef struct _thread_pool
{
  #ifdef WRITEBACK
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare[7];
  #else
  spinlock_t lock;
  #endif
  int16_t counter;
 
  struct _thread data[DNA_MAX_GROUP][DNA_MAX_THREAD];
  queue_t thread[DNA_MAX_GROUP];
}
thread_pool_t;

/*
 ****/

extern thread_pool_t thread_pool;
extern thread_handler_t APP_ENTRY_POINT;

extern status_t thread_alarm (void * data);

extern int32_t thread_idle (void * data);
extern int32_t thread_bootstrap (void * data);

#endif

