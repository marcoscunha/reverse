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
#include <Processor/Cache.h>


/****f* LifeCyclePrivate/core_initialize
 * SUMMARY
 * Initializes the core component.
 *
 * SYNOPSIS
 */

status_t core_start (void)

/*
 * FUNCTION
 * Start the core component.
 *
 * RETURN
 * * DNA_ERROR: no current thread on one of the CPU
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  uint32_t current_cpuid = cpu_mp_id ();
  thread_t thread = cpu_pool . cpu[current_cpuid] . current_thread;

  watch (status_t)
  {
    ensure (thread != NULL, DNA_ERROR);

    lock_acquire (& thread -> lock);

    if (current_cpuid == 0)
    {
      cpu_pool . cpu[current_cpuid] . status = DNA_CPU_RUNNING;
    }
    else
    {
      cpu_pool . cpu[current_cpuid] . status = DNA_CPU_READY;
      DCACHE_FLUSH(&cpu_pool.cpu[current_cpuid].status,sizeof(uint32_t));
      lock_acquire (& cpu_pool . queue . lock);
      queue_add (& cpu_pool . queue, & cpu_pool . cpu[current_cpuid]);
      lock_release (& cpu_pool . queue . lock);
    }

    thread -> info . status = DNA_THREAD_RUNNING;
    DCACHE_INVAL(&thread->context, CPU_CONTEXT_SIZE);
    cpu_context_load (& thread -> context);
    DCACHE_FLUSH(&thread->context, CPU_CONTEXT_SIZE);
    return DNA_OK;
  }
}

/*
 ****/

