/****h* CorePrivate/CpuPrivate
 * SUMMARY
 * Alarm management.
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

#ifndef DNA_CORE_CPU_PRIVATE_H
#define DNA_CORE_CPU_PRIVATE_H

#include <DnaTools/DnaTools.h>
#include <Private/Thread.h>

/****t* CpuPrivate/cpu_status_t
 * SUMMARY
 * CPU status type.
 *
 * SOURCE
 */

typedef enum _cpu_status
{
  DNA_CPU_READY     = 0xFACE,
  DNA_CPU_RUNNING   = 0xBEEF,
  DNA_CPU_DISABLED  = 0xDEAD
}
cpu_status_t;

/*
 ****/

/****t* CpuPrivate/cpu_t
 * SUMMARY
 * CPU type.
 *
 * SOURCE
 */

typedef struct _cpu
{ 
  queue_link_t link;

  int32_t id;
  cpu_status_t status;

  spinlock_t lock;
  spinlock_t ipi_lock;

  bigtime_t lap_date;
  queue_t * isr;

  thread_t current_thread;
  thread_t idle_thread;

  alarm_t current_alarm;
  queue_t alarm_queue;

  uint8_t * stack;
}
cpu_t;

/*
 ****/

/****t* CpuPrivate/cpu_pool_t
 * SUMMARY
 * CPU pool type.
 *
 * SOURCE
 */

typedef struct _cpu_pool
{
  spinlock_t lock;

  cpu_t * cpu;
  queue_t queue;
}
cpu_pool_t;

/*
 ****/

extern cpu_pool_t cpu_pool;

#endif

