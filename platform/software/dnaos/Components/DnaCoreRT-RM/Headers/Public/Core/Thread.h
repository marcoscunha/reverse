/****h* Core/Thread
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

#ifndef DNA_CORE_THREAD_PUBLIC_H
#define DNA_CORE_THREAD_PUBLIC_H

#include <stdint.h>
#include <Processor/Processor.h>

#include <Core/Alarm.h>
#include <DnaTools/DnaTools.h>

/****d* Thread/DNA_KERNEL_GROUP
 * SUMMARY
 * Thread kernel group constant.
 *
 * SOURCE
 */

#define DNA_KERNEL_GROUP  0

/*
 ****/

/****d* Thread/DNA_NO_AFFINITY
 * SUMMARY
 * Thread no-affinity constant.
 *
 * SOURCE
 */

#define DNA_NO_AFFINITY  0xFFFFFFFF

/*
 ****/

/****d* Thread/DNA_THREAD_DEFAULTS
 * SUMMARY
 * Default values for the thread_info structure.
 *
 * SOURCE
 */

#define DNA_THREAD_DEFAULTS \
{                           \
  "thread",                 \
  0,                        \
  DNA_KERNEL_GROUP,         \
  DNA_NO_AFFINITY,          \
  0,                        \
  DNA_NORMAL_THREAD,        \
  DNA_THREAD_SUSPENDED,     \
  DNA_NO_RESOURCE,          \
  -1,                       \
  0,                        \
  0,                        \
  NULL,                     \
  0x8000,                   \
  0,                        \
  0,                        \
  0                         \
};

/****t* Thread/thread_type_t
 * SUMMARY
 * Available thread type.
 *
 * SOURCE
 */

typedef enum _thread_type
{
  DNA_NORMAL_THREAD = 0,
  DNA_RT_THREAD
}
thread_type_t;

/*
 ****/

/****t* Thread/thread_status_t
 * SUMMARY
 * Available thread status.
 *
 * SOURCE
 */

typedef enum _thread_status
{
  DNA_THREAD_READY      = 0xFACE,
  DNA_THREAD_RUNNING    = 0xBEEF,
  DNA_THREAD_SLEEPING   = 0xBEDD,
  DNA_THREAD_SUSPENDED  = 0xD15A,
  DNA_THREAD_WAITING    = 0xB10C,
  DNA_THREAD_ENDED      = 0xDEAD
}
thread_status_t;

/*
 ****/

/****t* Thread/thread_resource_t
 * SUMMARY
 * Type of the resource for which a thread can wait.
 *
 * SOURCE
 */

typedef enum
{
  DNA_NO_RESOURCE,
  DNA_RESOURCE_SEMAPHORE,
  DNA_RESOURCE_THREAD
}
thread_resource_t;

/*
 ****/

/****t* Thread/thread_handler_t
 * SUMMARY
 * Thread handler type.
 *
 * SOURCE
 */

typedef int32_t (* thread_handler_t) (void * args);

/*
 ****/

/****t* Thread/thread_info_t
 * SUMMARY
 * Thread information type.
 *
 * SOURCE
 */

typedef struct _thread_info
{
  char name[DNA_NAME_LENGTH];

  int32_t cpu_id;
  int32_t group;
  int32_t affinity;
  int32_t sem_tokens;

  thread_type_t type;
  thread_status_t status;
  thread_resource_t resource;
  int32_t resource_id;

  bigtime_t kernel_time;
  bigtime_t user_time;

  void * stack_base;
  uint32_t stack_size;

  bigtime_t period;
  bigtime_t deadline;
  bigtime_t processing_time;
}
thread_info_t;

/*
 ****/

extern status_t thread_create (thread_handler_t handler,
    void * arguments, thread_info_t info, int32_t * tid);
extern status_t thread_destroy (int32_t id);

extern status_t thread_resume (int32_t id);
extern status_t thread_suspend (int32_t id);
extern status_t thread_yield (void);

extern status_t thread_wait (int32_t id, int32_t * value);
extern status_t thread_snooze (bigtime_t value);
extern void thread_exit (int32_t value);

extern status_t thread_find (char * name, int32_t * tid);
extern status_t thread_get_info (int32_t id, thread_info_t * info);

#endif

