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

/****f* Thread/thread_create
 * SUMMARY
 * create a new thread_t element.
 *
 * SYNOPSIS
 */

status_t thread_create (thread_handler_t handler, void * arguments,
    thread_info_t info, int32_t * tid)

/*
 * ARGUMENTS
 * * handler : the thread's handler
 * * arguments : the handler's arguments
 * * info : various thread information
 * * tid : the placeholder of the created thread's ID
 *
 * RESULT
 * * DNA_OUT_OF_MEM: cannot allocate memory to create a thread
 * * DNA_BAD_ARGUMENT: one of the arguments is invalid
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  int32_t index = 0;
  double thread_load = 0, expected_load = 0;
  thread_t thread = NULL;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (handler != NULL && tid != NULL, DNA_BAD_ARGUMENT);
    ensure (info . group >= 0 && info . group < DNA_MAX_GROUP, DNA_BAD_ARGUMENT);

    /*
     * Check the content of the information structure.
     */

    ensure (info . affinity == DNA_NO_AFFINITY || (info . affinity >= 0
          && info . affinity < cpu_mp_count ()), DNA_BAD_ARGUMENT);

    ensure (info . status == DNA_THREAD_SUSPENDED, DNA_BAD_ARGUMENT);
    ensure (info . resource == DNA_NO_RESOURCE, DNA_BAD_ARGUMENT);
    ensure (info . resource_id == -1, DNA_BAD_ARGUMENT);

    ensure ((info . deadline == 0 && info . period == 0) ||
        (info . deadline != 0 && info . period != 0), DNA_BAD_ARGUMENT);

    /*
     * Check the resource availability in case of a RT policy.
     * In the latter case, only the first CPU is checked, meaning
     * that it is only compatible with an UP/RT execution.
     */

    if (info . type == DNA_RT_THREAD)
    {
      ensure (info . affinity != DNA_NO_AFFINITY, DNA_BAD_ARGUMENT);

      thread_load = (double)info . processing_time / (double)info . period;
      expected_load = thread_load + scheduler . queue[info . affinity] . load;
      ensure (expected_load <= DNA_RT_LOAD_LIMIT, DNA_ERROR);

      scheduler . queue[info . affinity] . load += thread_load;
    }

    /*
     * Allocate the new thread structure.
     */

    thread = kernel_malloc (sizeof (struct _thread), true);
    ensure (thread != NULL, DNA_OUT_OF_MEM);

    /*
     * Deal with the thread group. In the future, only the kernel
     * and the application server will be able to create thread for
     * a different group than self.
     */

    thread -> id . s . group = info . group;
    thread -> priority = thread -> info . period;

    /*
     * Copy and adjust the information structure.
     */

    thread -> info = info;

    if (info . stack_base == NULL)
    {
      thread -> info . stack_base = kernel_malloc (info . stack_size, false);
      thread -> stack_allocated = true;
      check (error, thread -> info . stack_base != NULL, DNA_OUT_OF_MEM);
    }

    if (info . affinity == DNA_NO_AFFINITY)
    {
      thread -> info . affinity = cpu_mp_count ();
    }

    /*
     * Fill in the signature
     */

    thread -> signature . handler = handler;
    thread -> signature . arguments = arguments;

    /*
     * Initialize the context.
     */

    cpu_context_init (& thread -> context, thread -> info . stack_base,
        thread -> info . stack_size, thread_bootstrap, & thread -> signature);

    /*
     * Find a free thread slot
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& thread_pool . lock);

    for (index = 0; index < DNA_MAX_THREAD; index += 1)
    {
      if (thread_pool . thread[info . group][index] == NULL)
      {
        thread -> id . s . value = thread_pool . counter;
        thread -> id . s . index = index;

        thread_pool . thread[info . group][index] = thread;
        thread_pool . counter += 1;

        break;
      }
    }

    lock_release (& thread_pool . lock);
    cpu_trap_restore(it_status);

    check (error, index != DNA_MAX_THREAD, DNA_NO_MORE_THREAD);

    /*
     * Return the thread ID and success
     */

    *tid = thread -> id . raw;
    return DNA_OK;
  }

  rescue (error)
  {
    if (info . stack_base == NULL)
    {
      kernel_free (thread -> info . stack_base);
    }

    kernel_free (thread);
    leave;
  }
}

/*
 ****/
