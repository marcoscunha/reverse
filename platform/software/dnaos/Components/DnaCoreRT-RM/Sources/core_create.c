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
 * * Initialize the scheduler manager
 * * Initialize the cpu pool
 * * Initialize the alarm manager
 * * Initialize the semaphore pool
 * * Initialize the IDLE threads
 *
 * RETURN
 * * DNA_OK: the operation succeeded
 * * !DNA_OK: error creating a
 *
 * SOURCE
 */

{
  status_t status;
  cpu_t * cpu = NULL;
  void * area = NULL;
  thread_t thread;
  thread_id_t tid;
  thread_info_t thread_info = DNA_THREAD_DEFAULTS;

  watch (status_t)
  {
    /*
     * Initialize the thread pool
     */

    dna_memset (& thread_pool, 0, sizeof (thread_pool_t));

    area = kernel_malloc (DNA_MAX_GROUP * sizeof (thread_t *), true);
    thread_pool . thread = area;

    for (int32_t i = 0; i < DNA_MAX_GROUP; i += 1)
    {
      area = kernel_malloc (DNA_MAX_THREAD * sizeof (thread_t), true);
      thread_pool . thread[i] = area;
      check (thread_no_mem, area != NULL, DNA_OUT_OF_MEM);
    }

    /*
     * Initialize the scheduler
     */

    dna_memset (& scheduler, 0, sizeof (scheduler_t));

    area = kernel_malloc ((DNA_MAX_CPU + 1) * sizeof (scheduler_queue_t), true);
    scheduler . queue = area;
    check (thread_no_mem, area != NULL, DNA_OUT_OF_MEM);

    /*
     * Initialize the CPU pool
     */

    dna_memset (& cpu_pool, 0, sizeof (cpu_pool_t));
    area = kernel_malloc (DNA_MAX_CPU * sizeof (cpu_t), true);
    cpu_pool . cpu = area;
    check (cpu_no_mem, area != NULL, DNA_OUT_OF_MEM);

    /*
     * Initialize the alarm manager
     */

    dna_memset (& alarm_manager, 0, sizeof (alarm_manager_t));
    area = kernel_malloc (DNA_MAX_CPU * sizeof (cpu_t), true);
    alarm_manager . alarm = area;
    check (alarm_no_mem, area != NULL, DNA_OUT_OF_MEM);

    /*
     * Initialize the semaphore pool
     */

    dna_memset (& semaphore_pool, 0, sizeof (semaphore_pool_t));

    area = kernel_malloc (DNA_MAX_SEM * sizeof (semaphore_t), true);
    semaphore_pool . semaphore = area;
    check (sem_no_mem, area != NULL, DNA_OUT_OF_MEM);

    /*
     * Initialize the CPUs
     */

    for (int32_t cpu_i = 0; cpu_i < cpu_mp_count (); cpu_i++)
    {
      cpu = & cpu_pool . cpu[cpu_i];

      /*
       * Create the ISR lists
       */

      cpu -> isr = kernel_malloc (sizeof (queue_t) *
          cpu_trap_count (), true);
      check (cpu_initialize, area != NULL, DNA_OUT_OF_MEM);

      /*
       * Create the Idle stack
       */

      cpu -> stack = kernel_malloc (DNA_IDLE_STACK_SIZE, true);
      check (cpu_initialize, area != NULL, DNA_OUT_OF_MEM);

      /*
       * Create the Idle thread
       */

      dna_strcpy (thread_info . name, "IdleThread");
      thread_info . affinity = cpu_i;
      thread_info . stack_base = cpu -> stack;
      thread_info . stack_size = DNA_IDLE_STACK_SIZE;

      status = thread_create (thread_idle, NULL, thread_info, & tid . raw);
      check (cpu_initialize, status == DNA_OK, DNA_ERROR);

      thread = thread_pool . thread[tid . s . group][tid . s . index];
      thread -> info . status = DNA_THREAD_READY;

      /*
       * Deal with the new thread
       */

      cpu -> id = cpu_i;
      cpu -> status = DNA_CPU_DISABLED;
      cpu -> idle_thread = thread;
      cpu -> current_thread = thread;
    }

    /*
     * Create the main application thread. For now,
     * the application resides in the kernel.
     */

    dna_strcpy (thread_info . name, "Main");
    thread_info . affinity = DNA_NO_AFFINITY;
    thread_info . stack_base = NULL;
    thread_info . stack_size = DNA_THREAD_STACK_SIZE;

    status = thread_create (APP_ENTRY_POINT, NULL, thread_info, & tid . raw);
    check (cpu_initialize, status == DNA_OK, DNA_ERROR);

    thread = thread_pool . thread[tid . s . group][tid . s . index];
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

      if (cpu[cpu_i] . stack != NULL)
      {
        kernel_free (cpu[cpu_i] . stack);
      }

      if (cpu[cpu_i] . isr != NULL)
      {
        kernel_free (cpu[cpu_i] . isr);
      }
    }

    kernel_free (semaphore_pool . semaphore);
  }

  rescue (sem_no_mem)
  {
    kernel_free (alarm_manager . alarm);
  }

  rescue (alarm_no_mem)
  {
    kernel_free (cpu_pool . cpu);
  }

  rescue (cpu_no_mem)
  {
    kernel_free (scheduler . queue);
  }

  rescue (thread_no_mem)
  {
    for (int32_t i = 0; i < DNA_MAX_GROUP; i += 1)
    {
      if (thread_pool . thread[i] != NULL)
      {
        kernel_free (thread_pool . thread[i]);
      }
    }

    if (thread_pool . thread != NULL)
    {
      kernel_free (thread_pool . thread);
    }

    leave;
  }
}

/*
 ****/

