/****h* CorePrivate/SemaphorePrivate
 * SUMMARY
 * Semaphore management.
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

#ifndef DNA_CORE_SEMAPHORE_PRIVATE_H
#define DNA_CORE_SEMAPHORE_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

/****t* SemaphorePrivate/semaphore_id_t
 * SUMMARY
 * Semaphore ID type.
 *
 * SOURCE
 */

typedef union _semaphore_id
{
  int32_t raw;

  struct _semaphore_id_s
  {
    uint16_t value;
    uint16_t index;
  }
  s;
}
semaphore_id_t;

/*
 ****/

/****t* SemaphorePrivate/semaphore_t
 * SUMMARY
 * Semaphore type.
 *
 * SOURCE
 */

typedef struct _semaphore
{
  // XXX: Do not change the position of first element!
  queue_link_t link __attribute__((aligned(32)));
;
  semaphore_id_t id;
  #ifdef WRITEBACK 
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare[7];
  #else 
  spinlock_t lock;
  #endif
  queue_t waiting_queue;

  semaphore_info_t info;
}
* semaphore_t;

/*
 ****/

/****t* SemaphorePrivate/semaphore_pool_t
 * SUMMARY
 * Semaphore manager type.
 *
 * SOURCE
 */

typedef struct _semaphore_pool
{ 
  #ifdef WRITEBACK
  spinlock_t lock __attribute__((aligned(32)));
  int32_t spare[7];
  #else
  spinlock_t lock;
  #endif
  int16_t counter;
  struct _semaphore data[DNA_MAX_SEM];

  queue_t semaphore;
}
semaphore_pool_t;

/*
 ****/

extern semaphore_pool_t semaphore_pool;
extern status_t semaphore_alarm (void * data);

#endif

