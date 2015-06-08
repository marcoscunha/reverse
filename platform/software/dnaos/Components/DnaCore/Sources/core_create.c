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
#include <Processor/Processor.h>

/****f* LifeCyclePrivate/core_create
 * SUMMARY
 * Create the core component.
 *
 * SYNOPSIS
 */

status_t core_create (void)

/*
 * FUNCTION
 * * Initialize the thread pool
 * * Initialize the alarm manager
 * * Initialize the semaphore pool
 * * Initialize the IDLE threads
 *
 * RETURN
 * * DNA_OK: the operation succeeded
 * * !DNA_OK: error creating something
 *
 * SOURCE
 */

{
  volatile status_t status;
  cpu_t * cpu = NULL;
  thread_t thread;
  thread_id_t tid;
  thread_info_t thread_info;

  watch (status_t)
  {
    /*
     * Initialize the thread pool.
     */

    thread_pool . counter = 1;

    for (int32_t i = 0; i < DNA_MAX_GROUP; i += 1)
    {
      for (int32_t j = 0; j < DNA_MAX_THREAD; j += 1)
      {
        thread = & thread_pool . data[i][j];

        thread -> id . s . group = i;
        thread -> id . s . index = j;

        queue_add (& thread_pool . thread[i], thread);
      }
    }

    /*
     * Initialize the alarm pool.
     */

    alarm_pool . counter = 1;

    for (int32_t i = 0; i < DNA_MAX_ALARM; i += 1)
    {
      alarm_pool . data[i] . id . s . index = i;
      queue_add (& alarm_pool . alarm, & alarm_pool . data[i]);
    }

    /*
     * Initialize the semaphore pool.
     */

    semaphore_pool . counter = 1;

    for (int32_t i = 0; i < DNA_MAX_SEM; i += 1)
    {
      semaphore_pool . data[i] . id . s . index = i;
      queue_add (& semaphore_pool . semaphore, & semaphore_pool . data[i]);
    }

    /*
     * Initialize the port pool.
     */

    port_pool . counter = 1;

    for (int32_t i = 0; i < DNA_MAX_PORT; i += 1)
    {
      port_pool . data[i] . id . s . index = i;
      queue_add (& port_pool . port, & port_pool . data[i]);
    }

    /*
     * Initialize the CPUs
     */

    DNA_THREAD_SET_DEFAULTS (thread_info);

    for (int32_t cpu_i = 0; cpu_i < cpu_mp_count (); cpu_i++)
    {
      cpu = & cpu_pool . cpu[cpu_i];

      /*
       * Create the Idle thread
       */

      dna_strcpy (thread_info . name, "IdleThread");
      thread_info . affinity = cpu_i;
      thread_info . stack_base = NULL;
      thread_info . stack_size = DNA_IDLE_STACK_SIZE;
      DCACHE_FLUSH(&thread_info,sizeof(thread_info_t));

      status = thread_create (thread_idle, NULL, thread_info, & tid . raw);

      check (cpu_initialize, status == DNA_OK, DNA_ERROR);

      thread = & thread_pool . data[tid . s . group][tid . s . index];
      thread -> info . status = DNA_THREAD_READY;
      DCACHE_FLUSH(&thread->info.status,sizeof(thread_status_t));
      /*
       * Deal with the new thread
       */

      cpu -> id = cpu_i;
      DCACHE_FLUSH(&cpu->id, sizeof(int32_t));
      cpu -> status = DNA_CPU_DISABLED;
      DCACHE_FLUSH(&cpu->status, sizeof(cpu_status_t));
      cpu -> idle_thread = thread;
      DCACHE_FLUSH(&cpu->idle_thread, sizeof(thread_t));
      cpu -> current_thread = thread;
      DCACHE_FLUSH(&cpu->current_thread, sizeof(thread_t));
    }

    /*
     * Create the main application thread. For now,
     * the application resides in the kernel.
     */

    dna_strcpy (thread_info . name, "Main   "); // XXX: WRITEBACK ALIGNED
    thread_info . affinity = DNA_NO_AFFINITY;
    thread_info . stack_base = NULL;
    thread_info . stack_size = DNA_THREAD_STACK_SIZE;

    status = thread_create (APP_ENTRY_POINT, NULL, thread_info, & tid . raw);
    check (cpu_initialize, status == DNA_OK, DNA_ERROR);

    thread = & thread_pool . data[tid . s . group][tid . s . index];
    cpu_pool . cpu[0] . current_thread = thread;

    return DNA_OK;
  }

  rescue(cpu_initialize)
  {
    thread_t idle_thread = NULL;

    for (int32_t cpu_i = 0; cpu_i < cpu_mp_count (); cpu_i++)
    {
      idle_thread = cpu_pool . cpu[cpu_i] . idle_thread;

      if (idle_thread != NULL)
      {
        thread_destroy (idle_thread -> id . raw);

        cpu_pool . cpu[cpu_i] . idle_thread = NULL;
        cpu_pool . cpu[cpu_i] . current_thread = NULL;
      }
    }

    leave;
  }
}

/*
 * NOTE
 * No need to memset the areas of the pools, it is
 * done automatically when we erase the BSS.
 ****/

