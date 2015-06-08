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

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& thread_pool . lock);

    /*
     * Get an empty thread slot.
     */

    thread = queue_rem (& thread_pool . thread[info . group]);
    check (pool_error, thread != NULL, DNA_NO_MORE_SEM);

    /*
     * Make the place clean.
     */

    index = thread -> id . s . index;
    dna_memset (thread, 0, sizeof (struct _thread));

    thread -> id . s . index = index;
    thread -> id . s . value = thread_pool . counter;
    DCACHE_FLUSH(&thread->id,sizeof(thread_id_t));

    thread_pool . counter += 1;
    DCACHE_FLUSH(&thread_pool.counter ,sizeof(int16_t));

    /*
     * Release the pool.
     */

    lock_release (& thread_pool . lock);
    cpu_trap_restore(it_status);

    /*
     * Deal with the thread group. In the future, only the kernel
     * and the application server will be able to create thread for
     * a different group than self.
     */

    thread -> id . s . group = info . group;
    DCACHE_FLUSH(&thread->id,sizeof(thread_id_t));

    /*
     * Copy and adjust the information structure.
     */

    thread -> info = info;
    DCACHE_FLUSH(&thread->info, sizeof(thread_info_t));

    if (info . stack_base == NULL)
    {
      thread -> info . stack_base = kernel_malloc (info . stack_size, false);
      thread -> stack_allocated = true;
      DCACHE_FLUSH(&thread->info, sizeof(thread_info_t));
      DCACHE_FLUSH(&thread->stack_allocated, sizeof(bool));
      check (no_mem, thread -> info . stack_base != NULL, DNA_OUT_OF_MEM);
    }

    if (info . affinity == DNA_NO_AFFINITY)
    {
      thread -> info . affinity = cpu_mp_count ();
      DCACHE_FLUSH(&thread->info.affinity, sizeof(uint32_t));
    }

    /*
     * Fill in the signature
     */

    thread -> signature . handler = handler;
    thread -> signature . arguments = arguments;
    thread -> signature . it_restore = CPU_DEFAULT_ITRESTORE;
    DCACHE_FLUSH(&thread->signature, sizeof(thread_signature_t));

    /*
     * Initialize the context.
     */

    cpu_context_init (& thread -> context, thread -> info . stack_base,
        thread -> info . stack_size, thread_bootstrap, & thread -> signature);
    DCACHE_FLUSH(&thread->context,CPU_CONTEXT_SIZE);

    /*
     * Return the thread ID and success
     */

    *tid = thread -> id . raw;
    DCACHE_FLUSH(tid, sizeof(thread_id_t));
    return DNA_OK;
  }

  rescue (no_mem)
  {
    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& thread_pool . lock);

    thread -> id . s . value = 0;
    queue_add (& thread_pool . thread[info . group], thread);
  }

  rescue (pool_error)
  {
    cpu_trap_restore(it_status);
    lock_release (& thread_pool . lock);
    leave;
  }
}

/*
 ****/
