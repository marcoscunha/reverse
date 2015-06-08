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
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

/****f* Alarm/alarm_handler
 * SUMMARY
 * Time manager used for alarms.
 *
 * SYNOPSIS
 */

status_t alarm_handler (void)

/*
 * RESULT
 * * DNA_ERROR: their is no alarm to execute, or it is a false alarm
 * * DNA_INVOKE_SCHEDULER: success, the scheduler has to be invoked
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  queue_t alarm_garbage = DNA_QUEUE_DEFAULTS;
  alarm_t current_alarm = NULL, next_alarm = NULL;
  status_t status = DNA_OK;
  int32_t current_cpuid = cpu_mp_id ();
  bigtime_t start_time, quantum, current_deadline = 0, timer_delay;
  bigtime_t delta_start, delta_end;
  bool reschedule = false;
  bool process_next_alarm = true;
  cpu_t * cpu = & cpu_pool . cpu[current_cpuid];

  watch (status_t)
  {
    do
    {
      process_next_alarm = false;
      cpu_timer_get (cpu -> id, & start_time);

      /*
       * Lock the processor to access its queue and
       * its current alarm.
       */

      lock_acquire (& cpu -> lock);
      current_alarm = cpu -> current_alarm;

      /*
       * If the current alarm is valid, check if it
       * has to be restarted. Otherwise, it goes to the garbage.
       */

      check (no_alarm, current_alarm != NULL, DNA_ERROR);
      lock_acquire (& current_alarm -> lock);

      dna_log(VERBOSE_LEVEL, "Processing alarm (%d:%d) of thread 0x%x",
          current_alarm -> id . s . value, current_alarm -> id . s . index,
          current_alarm -> thread_id);

      current_deadline = current_alarm -> deadline;
      check (false_alarm, current_deadline <= start_time, DNA_ERROR);

      if (! current_alarm -> is_invalid
          && (current_alarm -> mode & DNA_PERIODIC_ALARM) != 0)
      {
        current_alarm -> deadline += current_alarm -> quantum;
        queue_insert (& cpu -> alarm_queue, alarm_comparator, current_alarm);
      }
      else
      {
        queue_add (& alarm_garbage, current_alarm);
      }

      lock_release (& current_alarm -> lock);

      /*
       * Look through the next alarm.
       */

      while ((next_alarm = queue_rem (& cpu -> alarm_queue)) != NULL)
      {
        if (! next_alarm -> is_invalid)
        {
          break;
        }

        queue_add (& alarm_garbage, current_alarm);
      }

      /*
       * Unlock the processor since we do not need its
       * data anymore.
       */
        
      cpu -> current_alarm = next_alarm;
      lock_release (& cpu -> lock);

      /*
       * Check if the next alarm is valid, and restart the
       * loop of its deadline is to close.
       */

      if (next_alarm != NULL)
      {
        /*
         * We get the new current time in order to adjust the alarm.
         * We also substract the time spent in the processor
         * handler. We might be off by approx. 500 cycles anyway,
         * due to the interrupt demultiplexer and the platform driver's
         * timer ISR.
         */

        cpu_timer_get (cpu -> id, & start_time);
        quantum = next_alarm -> deadline - start_time;
        timer_delay = start_time - current_deadline;

        if (quantum <= timer_delay + current_alarm -> execution_time)
        {
          dna_log(VERBOSE_LEVEL, "low (%d:%d), alarm (%d:%d) from thread 0x%x",
              (int32_t) quantum, (int32_t) timer_delay,
              next_alarm -> id . s . value, next_alarm -> id . s . index,
              next_alarm -> thread_id);

          process_next_alarm = true;
        }
        else
        {
          cpu_timer_set (cpu -> id, quantum);
        }
      }

      /*
       * Execute the alarm, and compute its WCET.
       */

      cpu_timer_get (cpu -> id, & delta_start);
      status = current_alarm -> callback (current_alarm -> data);
      cpu_timer_get (cpu -> id, & delta_end);

      quantum = delta_end - delta_start;
      if (quantum > current_alarm -> execution_time)
      {
        current_alarm -> execution_time = quantum;
      }

      /*
       * Update the reschedule flag.
       */

      reschedule = reschedule || status == DNA_INVOKE_SCHEDULER;
    }
    while (process_next_alarm);

    /*
     * Parse the garbage and delete its content.
     */

    while ((current_alarm = queue_rem (& alarm_garbage)) != NULL)
    {
      lock_acquire (& alarm_manager . lock);
      alarm_manager . alarm[current_alarm -> id . s . index] = NULL;
      lock_release (& alarm_manager . lock);

      kernel_free (current_alarm);
    }

    return reschedule ? DNA_INVOKE_SCHEDULER : DNA_OK;
  }

  rescue (false_alarm)
  {
    lock_release (& current_alarm -> lock);
    quantum = current_deadline - start_time;
    cpu_timer_set (cpu -> id, quantum);
  }

  rescue (no_alarm)
  {
    lock_release (& cpu -> lock);
    leave;
  }
}

/*
 ****/

