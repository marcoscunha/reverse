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
 * You should have received new_alarm copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Private/Core.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* Alarm/alarm_create
 * SUMMARY
 * Create an alarm.
 *
 * SYNOPSIS
 */

status_t alarm_create (bigtime_t quantum, alarm_mode_t mode,
    alarm_callback_t callback, void * data, int32_t * aid)

/*
 * ARGUMENTS
 * * quantum : time quantum of the alarm
 * * mode : the alarm's mode
 * * callback : the function to call when the alarm goes off
 * * data : the data associated to the function
 * * aid : the recipient of the new alarm's id
 *
 * RESULT
 * * DNA_BAD_ARGUMENT: one of the arguments is invalid
 * * DNA_OUT_OF_MEM: cannot allocate memory for the alarm
 * * DNA_ERROR: no more alarm slot or quantum too short
 * * DNA_OK: alarm created
 *
 * SOURCE
 */

{
  cpu_t * cpu = NULL;
  int16_t index;
  int32_t current_cpuid = 0;
  interrupt_status_t it_status;
  alarm_t new_alarm = NULL, old_alarm = NULL;
  bigtime_t start_time = 0, updated_time = 0, updated_quantum = 0;
  
  watch (status_t)
  {
    ensure (quantum != 0, DNA_BAD_ARGUMENT);

    /*
     * Deactivate interrupts and get current information.
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& alarm_pool . lock);

    current_cpuid = cpu_mp_id ();
    cpu = & cpu_pool . cpu[current_cpuid];

    new_alarm = queue_rem (& alarm_pool . alarm);
    check (no_alarm, new_alarm != NULL, DNA_NO_MORE_ALARM);

    /*
     * Make the place clean.
     */

    index = new_alarm -> id . s . index;
    dna_memset (new_alarm, 0, sizeof (struct _alarm));

    /*
     * Fill in the information.
     */

    new_alarm -> id . s . index = index;
    new_alarm -> id . s . value = alarm_pool . counter;

    alarm_pool . counter += 1;
    lock_release (& alarm_pool . lock);

    /*
     * Set various information.
     */

    new_alarm -> mode = mode;
    new_alarm -> thread_id = cpu -> current_thread -> id . raw;
    new_alarm -> cpu_id = current_cpuid;
    new_alarm -> is_invalid = false;

    new_alarm -> lock = 0;
    new_alarm -> execution_time = 0;

    new_alarm -> callback = callback;
    new_alarm -> data = data;

    /*
     * Check and compute the deadline according to the
     * alarm mode passed as parameter.
     */

    cpu_timer_get (current_cpuid, & start_time);

    switch (mode)
    {
      case DNA_PERIODIC_ALARM :
      case DNA_ONE_SHOT_RELATIVE_ALARM :
        {
          new_alarm -> quantum = quantum;
          new_alarm -> deadline = quantum + start_time;
          break;
        }

      case DNA_ONE_SHOT_ABSOLUTE_ALARM :
        {
          check (bad_quantum, quantum > start_time, DNA_BAD_ARGUMENT);

          new_alarm -> quantum = quantum - start_time;
          new_alarm -> deadline = quantum;
          break;
        }
    }

    /*
     * Deal with the new alarm
     */

    lock_acquire (& cpu -> lock);
    old_alarm = cpu -> current_alarm;

    if (cpu -> current_alarm == NULL ||
        old_alarm -> deadline > new_alarm -> deadline)
    {
dna_log(VERBOSE_LEVEL, "Set alarm (%d:%d)",
          new_alarm -> id . s . value, new_alarm -> id . s . index);

      /*
       * Check if we are still in the game, although
       * we spent time preparing the alarm. It should fail
       * on a 100Mhz RISC processos for an alarm < 20 usec.
       */

      cpu_timer_get (current_cpuid, & updated_time);
      updated_quantum = new_alarm -> deadline - updated_time;
      check (short_quantum, updated_quantum > 0, DNA_ERROR);

      /*
       * Cancel the old alarm, set the new one, and enqueue the old
       * alarm if this is necessary.
       */

      if (old_alarm != NULL)
      {
        cpu_timer_cancel (current_cpuid);
      }

      cpu -> current_alarm = new_alarm;
      cpu_timer_set (current_cpuid, updated_quantum);

      if (old_alarm != NULL)
      {
        queue_insert (& cpu -> alarm_queue, alarm_comparator, old_alarm);
      }
    }
    else
    {
dna_log(VERBOSE_LEVEL, "Enqueue alarm (%d:%d)",
          new_alarm -> id . s . value, new_alarm -> id . s . index);

      queue_insert (& cpu -> alarm_queue, alarm_comparator, new_alarm);
    }

    /*
     * Unlock everything and return the alarm.
     */

    lock_release (& cpu -> lock);
    cpu_trap_restore(it_status);

    *aid = new_alarm -> id . raw;
    return DNA_OK;
  }

  rescue (no_alarm)
  {
    lock_release (& alarm_pool . lock);
    cpu_trap_restore(it_status);
    leave;
  }

  rescue (short_quantum)
  {
    lock_release (& cpu -> lock);
  }

  rescue (bad_quantum)
  {
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

