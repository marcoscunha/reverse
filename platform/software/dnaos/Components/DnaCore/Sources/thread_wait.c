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
#include <Processor/Cache.h>


/****f* Thread/thread_wait
 * SUMMARY
 * Waits for a thread to complete.
 *
 * SYNOPSIS
 */

status_t thread_wait (int32_t id, int32_t * value)

/*
 * ARGUMENTS
 * * id : a thread identifier.
 * * value : a pointer to the return variable.
 *
 * RESULT
 * * DNA_BAD_ARGUMENT: on the arguments is invalid
 * * DNA_INVALID_THREAD_ID: the ID argument is not a valid thread ID
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  status_t status = DNA_OK;
  uint32_t current_cpuid = 0;
  thread_t self = NULL;
  thread_t thread = NULL, target = NULL;
  thread_id_t tid = { .raw = id };
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (tid . s . group >= 0, DNA_BAD_ARGUMENT);
    ensure (tid . s . group < DNA_MAX_GROUP, DNA_BAD_ARGUMENT);
    ensure (tid . s . index >= 0, DNA_BAD_ARGUMENT);
    ensure (tid . s . index < DNA_MAX_THREAD, DNA_BAD_ARGUMENT);

    /*
     * Get information about the current execution.
     */

    it_status = cpu_trap_mask_and_backup();
    current_cpuid = cpu_mp_id ();
    self = cpu_pool . cpu[current_cpuid] . current_thread;

    /*
     * Get the thread corresponding to ID.
     */

    lock_acquire (& thread_pool . lock);
    thread = & thread_pool . data[tid . s . group][tid . s . index];

    check (bad_thread, thread != NULL &&
        thread -> id . raw == tid . raw, DNA_INVALID_THREAD_ID);

    lock_acquire (& thread -> lock);
    lock_release (& thread_pool . lock);

    /*
     * If the thread is already dead, it is not necessary to
     * go through all the rescheduling pain
     */
    DCACHE_INVAL(&thread->info.status,sizeof(uint32_t));
    if (thread -> info . status != DNA_THREAD_ENDED)
    {
      lock_acquire (& thread -> wait . lock);
      lock_release (& thread -> lock);

      /*
       * If not, put ourselve in wait mode
       */

      lock_acquire (& self -> lock);


      self -> resource_queue = & thread -> wait;
      DCACHE_FLUSH(&self->resource_queue, sizeof(queue_t*));
      self -> info . status = DNA_THREAD_WAITING;
      DCACHE_FLUSH(&self->info.status, sizeof(thread_status_t));
      self -> info . resource = DNA_RESOURCE_THREAD;
      DCACHE_FLUSH(&self->info.resource,  sizeof(thread_resource_t));
      self -> info . resource_id = tid . raw;
      DCACHE_FLUSH(&self->info.resource_id,  sizeof(int32_t));


      /*
       * Elect a potential target thread.
       */

      status = scheduler_elect (& target, true);
      ensure (status != DNA_ERROR && status != DNA_BAD_ARGUMENT, status);

      /*
       * Add self into the ready queue.
       */
      TEST_ALIGN_LOCK("thread", &self->lock, &self->resource_queue);
      queue_add (& thread -> wait, self);

      /*
       * Reschedule self.
       */

      status = scheduler_switch (target, & thread -> wait);
      ensure (status == DNA_OK, status);

      /*
       * Check if the thread has not been destroyed.
       */

      lock_acquire (& thread_pool . lock);
      thread = & thread_pool . data[tid . s . group][tid . s . index];

      check (bad_thread, thread != NULL &&
        thread -> id . raw == tid . raw , DNA_THREAD_DESTROYED);

      lock_acquire (& thread -> lock);
      lock_release (& thread_pool . lock);

      /*
       * Check if we are here as a result of thread_suspend/thread_resume.
       */
      DCACHE_INVAL(&thread->info.status, sizeof(thread_status_t));
      check (bad_status,
          thread -> info . status == DNA_THREAD_ENDED, DNA_INTERRUPTED);
    }
    
    if (value != NULL)
    {
      *value = thread -> signature . return_value;
    }

    lock_release (& thread -> lock);
    cpu_trap_restore(it_status);
    return status;
  }

  rescue (bad_status)
  {
    lock_release (& thread -> lock);
    cpu_trap_restore(it_status);
    leave;
  }

  rescue (bad_thread)
  {
    lock_release (& thread_pool . lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

