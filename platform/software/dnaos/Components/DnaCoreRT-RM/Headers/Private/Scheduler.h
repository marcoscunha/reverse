/****h* CorePrivate/SchedulerPrivate
 * SUMMARY
 * Scheduler management.
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

#ifndef DNA_CORE_SCHEDULER_PRIVATE_H
#define DNA_CORE_SCHEDULER_PRIVATE_H

#include <stdbool.h>
#include <Private/Alarm.h>
#include <Private/Thread.h>

#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

#define DNA_RT_LOAD_LIMIT 0.63

/****t* SchedulerPrivate/scheduler_queue_t
 * SUMMARY
 * Scheduler queue type.
 *
 * SOURCE
 */

typedef struct _scheduler_queue
{
  queue_t normal;
  queue_t realtime;
  double load;
}
scheduler_queue_t;

/*
 ****/

/****t* SchedulerPrivate/scheduler_t
 * SUMMARY
 * Scheduler manager type.
 *
 * SOURCE
 */

typedef struct _scheduler
{
  spinlock_t lock;
  scheduler_queue_t * queue;
}
scheduler_t;

/*
 ****/

extern scheduler_t scheduler;

extern status_t scheduler_elect (thread_t * p_thread, bool with_idle);
extern status_t scheduler_dispatch (thread_t thread);
extern status_t scheduler_switch (thread_t thread, queue_t * queue);

extern status_t scheduler_alarm (void * data);
extern bool scheduler_comparator (void * item1, void * item2);

#endif

