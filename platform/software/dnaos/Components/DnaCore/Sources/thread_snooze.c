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

/****f* Thread/thread_snooze
 * SUMMARY
 * Snooze the current thread for a specified amount of time (ns).
 *
 * SYNOPSIS
 */

status_t thread_snooze (bigtime_t value)

/*
 * ARGUMENTS
 * * value : the number of microseconds to wait for
 *
 * RESULT
 * * DNA_ERROR: no more alarm slot or quantum too short
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  int32_t alarm_id = -1;
  uint32_t current_cpuid = 0;
  thread_t self = NULL;
  thread_t target = NULL;
  interrupt_status_t it_status = 0;
  status_t status = DNA_OK;

  watch (status_t)
  {
    /*
     * Disable interrupts and get current information
     */

    it_status = cpu_trap_mask_and_backup ();

    current_cpuid = cpu_mp_id();
    self = cpu_pool . cpu[current_cpuid] . current_thread;

    /*
     * Create the snooze alarm and elect a new thread.
     */

    status = alarm_create (value, DNA_ONE_SHOT_RELATIVE_ALARM,
        thread_alarm, self, & alarm_id);
    check (alarm_error, status == DNA_OK, status);

    status = scheduler_elect (& target, true);
    ensure (status != DNA_ERROR && status != DNA_BAD_ARGUMENT, status);

    /*
     * Update self information and switch context.
     */

    lock_acquire (& self -> lock);
    self -> info . status = DNA_THREAD_SLEEPING;

    status = scheduler_switch (target, NULL);
    ensure (status == DNA_OK, status);

    /*
     * Cancel the alarm, just in case we came back from
     * sleeping after a thread_suspend/thread_resume combination.
     */

    status = alarm_destroy (alarm_id);
    ensure (status != DNA_NO_TIMER && status != DNA_BAD_ARGUMENT, status);

    cpu_trap_restore (it_status);
    return (status == DNA_OK) ? DNA_INTERRUPTED : DNA_OK;
  }

  rescue (alarm_error)
  {
    cpu_trap_restore (it_status);
    leave;
  }
}

/*
 ****/

