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
#include <MemoryManager/MemoryManager.h>
#include <Processor/Processor.h>

/****f* Thread/thread_destroy
 * SUMMARY
 * Destroy a thread.
 *
 * SYNOPSIS
 */

status_t thread_destroy (int32_t id)

/*
 * ARGUMENTS
 * * id : a thread id.
 *
 * RESULT
 * * DNA_BAD_ARGUMENT: the id argument is invalid
 * * DNA_ERROR: the thread is neither suspended nor ended
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  uint32_t current_cpuid = 0;
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
     * Get some information about the execution.
     */

    it_status = cpu_trap_mask_and_backup ();
    current_cpuid = cpu_mp_id ();

    /*
     * Get the thread corresponding to ID and check the thread's status.
     */

    lock_acquire (& thread_pool . lock);
    thread = thread_pool . thread[tid . s . group][tid . s . index];

    check (bad_thread, thread != NULL &&
        thread -> id . raw == tid . raw , DNA_INVALID_THREAD_ID);

    lock_acquire (& thread -> lock);

    check (bad_status,
        thread -> info . status == DNA_THREAD_ENDED ||
        thread -> info . status == DNA_THREAD_SUSPENDED,
        DNA_ERROR);

    /*
     * Erase the thread's entry in the thread pool
     * and free its memory. Do an early unlock of the pool.
     */

    thread_pool . thread[tid . s . group][tid . s . index] = NULL;
    lock_release (& thread_pool . lock);

    lock_release (& thread -> lock);
    cpu_trap_restore (it_status);

    if (thread -> stack_allocated)
    {
      kernel_free (thread -> info . stack_base);
    }

    kernel_free (thread);
    return DNA_OK;
  }

  rescue (bad_status)
  {
    lock_release (& thread -> lock);
  }

  rescue (bad_thread)
  {
    lock_release (& thread_pool . lock);
    cpu_trap_restore (it_status);
    leave;
  }
}

/*
 ****/

