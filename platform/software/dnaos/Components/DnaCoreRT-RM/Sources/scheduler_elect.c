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

#include <Private/Core.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* SchedulerPrivate/scheduler_elect
 * SUMMARY
 * Elects a thread.
 *
 * SYNOPSIS
 */

status_t scheduler_elect (thread_t * p_thread, bool with_idle)

/*
 * ARGUMENTS
 * * p_thread : a pointer to a valid thread_t variable
 *
 * RESULT
 * * DNA_BAD_ARGUMENT: one of the arguments is invalid
 * * DNA_NO_AVAILABLE_THREAD: the available thread and with_idle is false
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  queue_t * queue = NULL;
  thread_t thread = NULL;
  int32_t current_cpuid = cpu_mp_id();

  watch (status_t)
  {
    ensure (p_thread != NULL, DNA_BAD_ARGUMENT);

    /*
     * Check the realtime local queue.
     */

    queue = & scheduler . queue[current_cpuid] . realtime;

    lock_acquire (& queue -> lock);
    thread = queue_rem (queue);

    check (thread_found, thread == NULL, DNA_OK);
    lock_release (& queue -> lock);

    /*
     * Check the normal local queue.
     */

    queue = & scheduler . queue[current_cpuid] . normal;

    lock_acquire (& queue -> lock);
    thread = queue_rem (queue);

    check (thread_found, thread == NULL, DNA_OK);
    lock_release (& queue -> lock);

    /*
     * Check the global queue.
     */

    queue = & scheduler . queue[cpu_mp_count ()] . normal;

    lock_acquire (& queue -> lock);
    thread = queue_rem (queue);

    check (thread_found, thread == NULL, DNA_OK);
    lock_release (& queue -> lock);

    /*
     * Return the IDLE thread if requested.
     */

    ensure (with_idle, DNA_NO_AVAILABLE_THREAD);

    thread = cpu_pool . cpu[current_cpuid] . idle_thread;
    check (thread_found, thread == NULL, DNA_OK);

    return DNA_ERROR;
  }

  rescue (thread_found)
  {
    lock_acquire (& thread -> lock);
    lock_release (& queue -> lock);

    *p_thread = thread;
    leave;
  }
}

/*
 ****/

