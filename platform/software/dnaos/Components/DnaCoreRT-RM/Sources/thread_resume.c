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

/****t* Thread/thread_resume
 * SUMMARY
 * Resume a thread.
 *
 * SYNOPSIS
 */

status_t thread_resume (int32_t id)

/*
 * ARGUMENTS
 * * id : a thread id.
 *
 * RESULT
 * * DNA_BAD_ARGUMENT: the argument ID is invalid
 * * DNA_INVALID_THREAD_ID: the argument ID is not a valid thread ID
 * * DNA_NO_ERROR: success, the thread has been enqueued
 * * DNA_OK: success, the thread has been rescheduled
 *
 * SOURCE
 */

{
  status_t status = DNA_OK;
  thread_t thread;
  thread_id_t tid = { .raw = id };
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (tid . s . group >= 0, DNA_BAD_ARGUMENT);
    ensure (tid . s . group < DNA_MAX_GROUP, DNA_BAD_ARGUMENT);
    ensure (tid . s . index >= 0, DNA_BAD_ARGUMENT);
    ensure (tid . s . index < DNA_MAX_THREAD, DNA_BAD_ARGUMENT);

    /*
     * Get the thread corresponding to the current id
     * and check the thread's status.
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& thread_pool . lock);

    thread = thread_pool . thread[tid . s . group][tid . s . index];

    check (bad_thread, thread != NULL &&
        thread -> id . raw == tid . raw , DNA_INVALID_THREAD_ID);

    lock_acquire (& thread -> lock);
    lock_release (& thread_pool . lock);

    check (bad_status,
        thread -> info . status == DNA_THREAD_SUSPENDED,
        DNA_ERROR);

    /*
     * Create the thread's periodic alarm, if necessary.
     */

    if (thread -> info . type == DNA_RT_THREAD)
    {
      alarm_create (thread -> info . period, DNA_PERIODIC_ALARM,
          scheduler_alarm, thread, & thread -> periodic_alarm);
    }

    /*
     * Update the thread information and resume it.
     */

    thread -> info . status = DNA_THREAD_READY;
    status = scheduler_dispatch (thread);

    /*
     * Note for later: we DO NOT call thread yield here.
     * Reason: it would only favorize the FIFO scheduling method.
     * Latency should NOT appear on RR or RT scheduling methods.
     * We return the dispatch status anyway, so the user can adapt
     * its code accordingly, and call thread_yield himself.
     */

    cpu_trap_restore (it_status);
    return status == DNA_INVOKE_SCHEDULER ? DNA_NO_ERROR : DNA_OK;
  }

  rescue (bad_status)
  {
    lock_release (& thread -> lock);
  }

  rescue (bad_thread)
  {
    cpu_trap_restore (it_status);
    lock_release (& thread_pool . lock);
    leave;
  }
}

/*
 ****/

