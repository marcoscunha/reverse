/****h* Tools/Queue
 * SUMMARY
 * Queue management.
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

#ifndef DNA_TOOLS_QUEUE_H
#define DNA_TOOLS_QUEUE_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <Core/Lock.h>
#include <DnaTools/Status.h>

/****d* Queue/DNA_QUEUE_DEFAULTS
 * SUMMARY
 * Queue default values.
 *
 * SOURCE
 */

#define DNA_QUEUE_SET_DEFAULTS(_q)  \
{                                   \
  _q . lock = 0;                    \
  _q . head = NULL;                 \
  _q . tail = NULL;                 \
  _q . status = 0;                  \
}

/*
 ****/

/****t* Queue/queue_inspector_t
 * SUMMARY
 * Queue inspector type.
 *
 * SOURCE
 */

typedef bool (* queue_inspector_t)(void * item, va_list list); 

/*
 ****/

/****t* Queue/queue_comparator_t
 * SUMMARY
 * Queue comparator type. Represent an ordering < function.
 *
 * SOURCE
 */

typedef bool (* queue_comparator_t)(void * item1, void * item2); 

/*
 ****/

/****t* Queue/queue_link_t
 * SUMMARY
 * Queue item type.
 *
 * SOURCE
 */

typedef struct _queue_link
{
  struct _queue_link * next;
}
queue_link_t;

/*
 ****/

/****t* Queue/queue_t
 * SUMMARY
 * Queue type.
 *
 * SOURCE
 */

typedef struct queue
{
#ifdef WRITEBACK
  spinlock_t     lock __attribute__((aligned(32)));
  uint32_t       spare1[7];
#else
  spinlock_t lock;
#endif
  queue_link_t * head;
  uint32_t       spare2[7];
  queue_link_t * tail;
  uint32_t       spare3[7];
  volatile uint32_t status;
  uint32_t       spare4[7];
}
queue_t;

/*
 ****/
#define TEST_ALIGN_LOCK(_s,_lock, _next_field) if( (uintptr_t)_lock & 0x1F) dna_log(PANIC_LEVEL, "@@@@@@@@@@@@@@ %s l %p\n", _s, _lock);\
                                                  if( (uintptr_t)_next_field & 0x1F) dna_log(PANIC_LEVEL, "@@@@@@@@@@%s n %p\n", _s, _next_field);
 


extern status_t queue_add (queue_t * queue, void * data);
extern void * queue_rem (queue_t * queue);

extern status_t queue_extract (queue_t * queue, void * data);
extern status_t queue_pushback (queue_t * queue, void * data);

extern void * queue_lookup (queue_t * queue, queue_inspector_t inspector, ...);
extern void queue_walk (queue_t * queue, queue_inspector_t inspector, ...);

extern void queue_insert (queue_t * queue,
    queue_comparator_t comparator, void * data);

#endif

