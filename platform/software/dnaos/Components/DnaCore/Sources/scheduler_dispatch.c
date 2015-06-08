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

/****f* SchedulerPrivate/scheduler_dispatch
 * SUMMARY
 * Dispatch a thread.
 *
 * SYNOPSIS
 */

status_t scheduler_dispatch (thread_t thread)

/*
 * ARGUMENTS
 * * A valid thread_t 
 *
 * FUNCTION
 * In the first part of the test, we check whether a distant, compatible CPU is
 * available. If not, if the thread is compatible with the current processor,
 * and whether this processor is available or not, we return that it is
 * necessary to invoke the scheduler. It will be the role of the calling
 * function to decide what to do.
 *
 * RETURN:
 * * DNA_BAD_ARGUMENT: thread is not valid
 * * DNA_INVOKE_SCHEDULER: success, invoke the scheduler on return
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  cpu_t * cpu = NULL;
  status_t status = DNA_OK;
  int32_t affinity = thread -> info . affinity;

  watch (status_t)
  {
    ensure (thread != NULL, DNA_BAD_ARGUMENT);

    /*
     * Look for an available processor.
     */

    lock_acquire (& cpu_pool . queue . lock);

    if (affinity == cpu_mp_count ())
    {
      cpu = queue_rem (& cpu_pool . queue);
    }
    else
    {
      status = queue_extract (& cpu_pool . queue, & cpu_pool . cpu[affinity]);
      cpu = status == DNA_OK ? & cpu_pool . cpu[affinity] : NULL;
    }

    lock_release (& cpu_pool . queue . lock);

    /*
     * Check if we can send the thread to a distant CPU.
     */
    DCACHE_INVAL(&cpu->id, sizeof(int32_t));
    if (cpu != NULL && cpu -> id != cpu_mp_id ())
    {
dna_log(INFO_LEVEL, "(%d) %s => CPU(%d)", cpu_mp_id (),
          thread -> info . name, cpu -> id);
      
      lock_acquire (& cpu -> ipi_lock);
      cpu_mp_send_ipi (cpu -> id, DNA_IPI_DISPATCH, thread);
    }
    else
    {
      lock_acquire (& scheduler . queue[affinity] . lock);
      queue_add (& scheduler . queue[affinity], thread);

      lock_release (& scheduler . queue[affinity] . lock);
      lock_release (& thread -> lock);

      /*
       * If the thread is compatible with the current CPU,
       * we return DNA_INVOKE_SCHEDULER to indicate potential reschedule.
       */

      if (affinity == cpu_mp_count () || affinity == cpu_mp_id ())
      {
dna_log(INFO_LEVEL, "%s => Q(%d)", thread -> info . name, affinity);
        status = DNA_INVOKE_SCHEDULER;
      }
    }

    return status;
  }
}

/*
 * NOTES
 * Interrupts must be disabled.
 ****/

