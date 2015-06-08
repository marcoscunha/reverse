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

#include <stdbool.h>
#include <Private/Core.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>
#include <Processor/Cache.h>


/****f* Semaphore/semaphore_acquire
 * SUMMARY
 * Acquire a semaphore.
 *
 * SYNOPSIS
 */

status_t semaphore_acquire (int32_t id, int32_t tokens,
    int32_t flags, bigtime_t timeout)

/*
 * ARGUMENTS
 * * id : the ID of the semaphore to acquire.
 * * tokens : the number of tokens to acquire
 * * flags : flags of the operation.
 * * timeout : time, in nanosecond, when the acquire process must abort.
 *
 * RESULT
 * * DNA_BAD_ARGUMENT if the tokens parameter is invalid.
 * * DNA_BAD_SEM_ID if the sid parameter is invalid.
 * * DNA_WOULD_BLOCK if the semaphore is already locked and the timeout is 0.
 * * DNA_SEM_DESTROYED if the semaphore has been deleted during a wait.
 * * DNA_INTERRUPTED if the acquisition has been interrupted.
 * * DNA_TIMED_OUT if the acquisition has timed out.
 * * DNA_OK if the operation succeded.
 *
 * SOURCE
 */

{
  bool has_alarm = false, can_interrupt = false;
  thread_t self = NULL, thread = NULL;
  semaphore_t sem = NULL;
  semaphore_id_t sid = { .raw = id };
  status_t status = DNA_OK, alarm_status = DNA_OK;
  int32_t alarm, rem_tokens;
  alarm_mode_t alarm_mode = DNA_ONE_SHOT_RELATIVE_ALARM;;
  uint32_t current_cpuid = 0;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (tokens > 0, DNA_BAD_ARGUMENT);
    ensure (sid . s . index < DNA_MAX_SEM, DNA_BAD_SEM_ID);

    /*
     * Check the flags for a timeout parameter.
     */

    if ((flags & DNA_RELATIVE_TIMEOUT) != 0)
    {
      has_alarm = true;
      alarm_mode = DNA_ONE_SHOT_RELATIVE_ALARM;
    }
    else if ((flags & DNA_ABSOLUTE_TIMEOUT) != 0)
    {
      has_alarm = true;
      alarm_mode = DNA_ONE_SHOT_ABSOLUTE_ALARM;
    }

    ensure (! has_alarm || (has_alarm && timeout >= 0), DNA_BAD_ARGUMENT);

    /*
     * Check the flags to discover if the semaphore can be interrupted.
     */

    can_interrupt = ((flags & DNA_CAN_INTERRUPT) != 0);

    /*
     * Disable the interrupts, and get the current
     * execution environment (cpuid, self_id).
     */

    it_status = cpu_trap_mask_and_backup();
    current_cpuid = cpu_mp_id();
    self = cpu_pool . cpu[current_cpuid] . current_thread;

    /*
     * Look for the semaphore with ID sid.
     */
    lock_acquire (& semaphore_pool . lock);

    sem = & semaphore_pool . data[sid . s . index];
    DCACHE_FLUSH(&sem,sizeof(semaphore_t*));
    check (bad_semid, sem != NULL, DNA_BAD_SEM_ID);
    check (bad_semid, sem -> id . raw == sid . raw, DNA_BAD_SEM_ID);
    

   


dna_log(VERBOSE_LEVEL, "%s(#%d), %d:%d(#%d)", self -> info . name,
        tokens, sid . s . value, sid . s . index, sem -> info . tokens);
    lock_acquire (& sem -> lock);
    lock_release (& semaphore_pool . lock);

    /*
     * Remove the necessary tokens and decide what
     * to do next depending on the result.
     */
    DCACHE_INVAL(&(sem->info.tokens), sizeof(int32_t));
    rem_tokens = sem -> info . tokens - tokens;
    DCACHE_FLUSH(&rem_tokens,sizeof(int32_t));

    if (rem_tokens >= 0)
    {
      /*
       * Everything is OK, take the requested tokens.
       */
      DCACHE_INVAL(&sem->info.tokens,sizeof(int32_t));
      sem -> info . tokens -= tokens;
      DCACHE_FLUSH(&sem->info.tokens,sizeof(int32_t));
    }
    else
    {
      check (quick_abort, (! has_alarm) ||
          (has_alarm && timeout != 0), DNA_WOULD_BLOCK);

      /*
       * Not enough tokens. Set the sem -> tokens to 0.
       * Create the alarm if necessary.
       */
      sem -> info . tokens = 0;
      DCACHE_FLUSH(&sem->info.tokens, sizeof(uint32_t));

      if (has_alarm)
      {
        status = alarm_create (timeout, alarm_mode,
            semaphore_alarm, self, & alarm);
        check (quick_abort, status == DNA_OK, status);
      }

      /*
       * Update self information.
       */

      lock_acquire (& self -> lock);

      if (can_interrupt || has_alarm)
      {
        self -> resource_queue = & sem -> waiting_queue;
      }
      else
      {
        self -> resource_queue = NULL;
        DCACHE_FLUSH(&self->resource_queue,sizeof(queue_t *));
      }

      self -> info . sem_tokens = -1 * rem_tokens;
      DCACHE_FLUSH(&self->info.sem_tokens, sizeof(int32_t));

      DCACHE_INVAL(&self->info.status,sizeof(thread_status_t));
      self -> info . status = DNA_THREAD_WAITING;
      DCACHE_FLUSH(&self->info.status,sizeof(thread_status_t));

      self -> info . resource = DNA_RESOURCE_SEMAPHORE;
      DCACHE_FLUSH(&self->info.resource, sizeof(thread_resource_t));

      DCACHE_INVAL(&sem->id.raw,sizeof(int32_t));
      self -> info . resource_id = sem -> id . raw;
      DCACHE_FLUSH(&self->info.resource_id,sizeof(int32_t));

      /*
       * Look for a potential target thread.
       */

      lock_acquire (& sem -> waiting_queue . lock);
      lock_release (& sem -> lock);

      dna_log(VERBOSE_LEVEL, "%s(#%d) DNA_NO_RESOURCE\n", self -> info . name, self->info.resource);

      status = scheduler_elect (& thread, true);
      ensure (status != DNA_ERROR && status != DNA_BAD_ARGUMENT, status);

      /*
       * Add self in the ready queue.
       */

      queue_add (& sem -> waiting_queue, self);

      /*
       * Reschedule self.
       */
      status = scheduler_switch (thread, & sem -> waiting_queue);
      ensure (status == DNA_OK, status);

      /*
       * Check if the semaphore still exist.
       */
      lock_acquire (& semaphore_pool . lock);

      sem = & semaphore_pool . data[sid . s . index];
      check (bad_semid, sem != NULL, DNA_SEM_DESTROYED);
      check (bad_semid, sem -> id . raw == sid . raw, DNA_SEM_DESTROYED);

      lock_acquire (& sem -> lock);
      lock_release (& semaphore_pool . lock);

      /*
       * If there is an alarm programmed, cancel it.
       */

      if (has_alarm)
      {
        alarm_status = alarm_destroy (alarm);
        check (quick_abort, status != DNA_NO_TIMER, status);
        check (quick_abort, status != DNA_BAD_ARGUMENT, status);
      }

      dna_log(VERBOSE_LEVEL, " %s(#%d) DNA_NO_RESOURCE", self-> info . name, self->info.resource);


      /*
       * Check if we have not been interrupted with a
       * thread_suspend/thread_resume combination.
       */
       DCACHE_INVAL(&self->info.resource, sizeof(thread_resource_t));
       check (acquire_interrupted,
            self -> info . resource == DNA_NO_RESOURCE, DNA_INTERRUPTED);

      /*
       * Although a status equal to DNA_UNKNOWN_ALARM means that the alarm
       * has already been fired, the semaphore could perfectly have been
       * V()-ed in the meantime. Hence, we need to recheck the sem tokens.
       */

      if (has_alarm && alarm_status == DNA_UNKNOWN_ALARM)
      {
        check (acquire_timed_out,
            sem -> info . tokens >= tokens, DNA_TIMED_OUT);
        sem -> info . tokens -= tokens;
      }

      status = DNA_OK;
    }

    /*
     * If the acquire succeeded, set the latest holder as self.
     */
    
    if (status == DNA_OK)
    {
      DCACHE_INVAL(&self->id, sizeof(thread_id_t));
      sem -> info . latest_holder = self -> id . raw;
      DCACHE_FLUSH(&sem->info.latest_holder, sizeof(int32_t));
    }

    lock_release (& sem -> lock);
    cpu_trap_restore(it_status);
    return status;
  }

  rescue (bad_semid)
  {
    lock_release (& semaphore_pool . lock);
    cpu_trap_restore(it_status);
    leave;
  }

  rescue (quick_abort)
  {
    lock_release (& sem -> lock);
    cpu_trap_restore(it_status);
    leave;
  }

  rescue (acquire_interrupted)
  {
    lock_acquire (& self -> lock);

/*    dna_log(VERBOSE_LEVEL, "acquire_interrupted %s(#%d), %d:%d(#%d) DNA_NO_RESOURCE ", self -> info . name,
        tokens, sid . s . value, sid . s . index, sem -> info . tokens);
        */

    self -> resource_queue = NULL;
    self -> info . sem_tokens = 0;
    self -> info . resource = DNA_NO_RESOURCE;
  
    self -> info . resource_id = -1;
    lock_release (& self -> lock);

    lock_release (& sem -> lock);
    cpu_trap_restore(it_status);
    leave;
  }

  rescue (acquire_timed_out)
  {
    queue_extract (& sem -> waiting_queue, self);
    sem -> info . tokens = rem_tokens + tokens;
    lock_release (& sem -> lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

