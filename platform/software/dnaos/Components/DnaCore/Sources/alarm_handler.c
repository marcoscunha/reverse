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
  queue_t alarm_garbage;
  alarm_t current_alarm = NULL, next_alarm = NULL;
  status_t status = DNA_OK;
  int32_t current_cpuid = cpu_mp_id (), liveness = 10;
  bigtime_t start_time, quantum, current_deadline = 0, timer_delay;
  bigtime_t delta_start, delta_end, expected_deadline, expected_delay;
  volatile bool reschedule = false;
  bool process_next_alarm = true;
  cpu_t * cpu = & cpu_pool . cpu[current_cpuid];

  watch (status_t)
  {
    DNA_QUEUE_SET_DEFAULTS (alarm_garbage);

    do
    {
      cpu_timer_get (cpu -> id, & start_time);

      /*
       * Lock the processor to access its queue and
       * its current alarm.
       */

      lock_acquire (& cpu -> lock);
      current_alarm = cpu -> current_alarm;

      /*
       * Check if the current alarm is valid.
       */

      check (no_alarm, current_alarm != NULL, DNA_ERROR);
      lock_acquire (& current_alarm -> lock);

dna_log(VERBOSE_LEVEL, "(%d:%d), %lld/%lld, of 0x%x @ %lld",
          current_alarm -> id . s . value, current_alarm -> id . s . index,
          current_alarm -> deadline, current_alarm -> quantum,
          current_alarm -> thread_id, start_time);

      current_deadline = current_alarm -> deadline;

      check (false_alarm, process_next_alarm ||
          current_deadline < start_time, DNA_ERROR);

      process_next_alarm = false;

      /*
       * Check if the alarm is valid. If so, check if
       * it is periodical and restart it if necessary.
       */

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
       * Look through the next alarm if no reschedule is needed.
       */

      while ((next_alarm = queue_rem (& cpu -> alarm_queue)) != NULL)
      {
        lock_acquire (& next_alarm -> lock);

        if (! next_alarm -> is_invalid)
        {
          lock_release (& next_alarm -> lock);
          break;
        }

        queue_add (& alarm_garbage, next_alarm);
        lock_release (& next_alarm -> lock);
      }

      /*
       * Unlock the processor since we do not need its
       * data anymore.
       */
        
      cpu -> current_alarm = next_alarm;
      lock_release (& cpu -> lock);

      /*
       * Check if the next alarm is valid, and restart the
       * loop if its deadline is to close.
       */

      if (next_alarm != NULL)
      {
        /*
         * If we are dealing with the same alarm as the current one,
         * we need to decrease the liveness counter.
         */

        liveness = current_alarm == next_alarm ? liveness - 1 : 10;

        if (liveness == 0)
        {
dna_log(PANIC_LEVEL,
              "Can't stabilize alarm (%d:%d) after 10 cycle, abandon.",
              next_alarm -> id . s . value, next_alarm -> id . s . index);

          lock_acquire (& next_alarm -> lock);
          next_alarm -> is_invalid = next_alarm -> is_invalid || liveness == 0;
          lock_release (& next_alarm -> lock);
        }

        /*
         * Compute the delays and the expected deadline.
         */

        timer_delay = start_time - current_deadline;
        expected_deadline = start_time + timer_delay
          + current_alarm -> execution_time;
        expected_delay = expected_deadline - next_alarm -> deadline;

        /*
         * If the deadline is way out of base, restart the loop,
         * otherwise program the timer.
         */

        if (expected_delay > 0)
        {
dna_log(VERBOSE_LEVEL, "Over by %d ns, alarm (%d:%d) from thread 0x%x.",
              (int32_t) expected_delay, next_alarm -> id . s . value,
              next_alarm -> id . s . index, next_alarm -> thread_id);

          process_next_alarm = true;
        }
        else
        {
          cpu_timer_set (cpu -> id, -1 * expected_delay);
        }
      }

      /*
       * Execute the alarm, and compute its WCET.
	   *    if and only if it is still valid.
       */
	  if (!current_alarm->is_invalid){
		   cpu_timer_get (cpu -> id, & delta_start);
		   status = current_alarm -> callback (current_alarm -> data);
		   cpu_timer_get (cpu -> id, & delta_end);

		   quantum = delta_end - delta_start;
		   if (quantum > current_alarm -> execution_time)
		   {
				current_alarm -> execution_time = quantum;
		   }
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
      current_alarm -> id . s . value = 0;

      lock_acquire (& alarm_pool . lock);
      dna_printf("0x%08x alarm_pool.lock %d %s\n",&alarm_pool.lock, alarm_pool.lock, __func__ );
      queue_add (& alarm_pool . alarm, current_alarm);
      lock_release (& alarm_pool . lock);
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

