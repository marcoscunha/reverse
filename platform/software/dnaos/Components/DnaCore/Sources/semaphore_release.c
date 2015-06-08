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


/****f* Semaphore/semaphore_release
 * SUMMARY
 * Release a semaphore.
 *
 * SYNOPSIS
 */

status_t semaphore_release (int32_t id, int32_t tokens, int32_t flags)

/*
 * ARGUMENTS
 * * id : a semaphore ID
 * * tokens : the number of tokens to release
 * * flags : the flags of the operation
 *
 * RESULT
 * * DNA_BAD_SEM_ID: the argument id is invalid
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  thread_t thread = NULL;
  semaphore_t sem = NULL;
  semaphore_id_t sid = { .raw = id };
  interrupt_status_t it_status = 0;
  status_t status = DNA_OK;
  bool smart_to_reschedule = false;

  watch (status_t)
  {
    ensure (tokens > 0, DNA_BAD_ARGUMENT);
    ensure (sid . s . index < DNA_MAX_SEM, DNA_BAD_SEM_ID);

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& semaphore_pool . lock);

    /*
     * Look for the semaphore with ID id
     */

    sem = & semaphore_pool . data[sid . s . index];
    check (invalid_semaphore, sem != NULL, DNA_BAD_SEM_ID);
    DCACHE_INVAL(&sem -> id . raw, sizeof(int32_t));
    check (invalid_semaphore, sem -> id . raw == sid . raw, DNA_BAD_SEM_ID);

dna_log(VERBOSE_LEVEL, "%d tokens on ID(%d:%d) TOKEN(%d)",
        tokens, sid . s . value,
        sid . s . index, sem -> info . tokens);
    lock_acquire (& sem -> lock);
    lock_release (& semaphore_pool . lock);

    /*
     * Decide what to do according to the number
     * of tokens required by a potential waiting thread
     */

    lock_acquire (& sem -> waiting_queue . lock);

    while (tokens != 0)
    {
      /*
       * Check if a thread is available.
       */

      if ((thread = queue_rem (& sem -> waiting_queue)) == NULL)
      {
        break;
      }

      /*
       * Lock the thread and check its status.
       */
      lock_acquire (& thread -> lock);

      DCACHE_INVAL((&thread->info.status),sizeof(uint32_t));    
      check (invalid_thread_status,
          thread -> info . status == DNA_THREAD_WAITING,
          DNA_ERROR);

      /*
       * Check the number of tokens it requests.
       */
      DCACHE_INVAL(&thread->info.sem_tokens, sizeof(uint32_t));
      if (thread -> info . sem_tokens <= tokens)
      {
       tokens -= thread -> info . sem_tokens;

        thread -> resource_queue = NULL;
        DCACHE_FLUSH(&thread->resource_queue, sizeof(queue_t*));
        thread -> info . sem_tokens = 0;
        DCACHE_FLUSH(&thread->info.sem_tokens, sizeof(int32_t));
        thread -> info . resource = DNA_NO_RESOURCE;
        DCACHE_FLUSH(&thread->info.resource, sizeof(thread_resource_t));

        dna_log(VERBOSE_LEVEL, " %s  DNA_NO_RESOURCE ", thread-> info . name);

        thread -> info . resource_id = -1;
        DCACHE_FLUSH(&thread->info.resource_id, sizeof(int32_t));
        thread -> info . status = DNA_THREAD_READY;
        DCACHE_FLUSH(&thread->info.status, sizeof(thread_status_t));

        status = scheduler_dispatch (thread);

        smart_to_reschedule = smart_to_reschedule ||
          (status == DNA_INVOKE_SCHEDULER);
      }
      else
      {
        thread -> info . sem_tokens -= tokens;
        lock_release (& thread -> lock);

        tokens = 0;
        queue_pushback (& sem -> waiting_queue, thread);

        break;
      }
    }
    lock_release (& sem -> waiting_queue . lock);
 
    /*
     * Add the remaining number of token
     * and release the sem lock
     */
    {
    DCACHE_INVAL(&sem->info.tokens, sizeof(uint32_t));
    sem -> info . tokens += tokens;
    DCACHE_FLUSH(&sem->info.tokens, sizeof(uint32_t));
    }

    lock_release (& sem -> lock);
    cpu_trap_restore(it_status);

    /*
     * Now we deal with the reschedule
     */

    if ((flags & DNA_NO_RESCHEDULE) == 0 && smart_to_reschedule)
    {
      thread_yield ();
    }

    return DNA_OK;
  }

  rescue (invalid_thread_status)
  {
    lock_release (& thread -> lock);
    lock_release (& sem -> waiting_queue . lock);
    lock_release (& sem -> lock);

    cpu_trap_restore(it_status);
    leave;
  }

  rescue (invalid_semaphore)
  {
    lock_release (& semaphore_pool . lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

