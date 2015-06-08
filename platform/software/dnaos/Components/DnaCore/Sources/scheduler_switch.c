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

/****f* SchedulerPrivate/scheduler_switch
 * SUMMARY
 * Switches contexts between a thread and the current running thread.
 *
 * SYNOPSIS
 */

status_t scheduler_switch (thread_t thread, queue_t * queue)

/*
 * ARGUMENTS
 * * thread : a valid ready thread
 * * queue : the waiting queue where the current thread has to be stored
 *
 * RETURN
 * * DNA_BAD_ARGUMENT: thread is not valid
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  uint32_t current_cpuid = cpu_mp_id ();
  bigtime_t current_time = 0, delta = 0;
  extern uint32_t __scheduler_switch_end;
  cpu_t * cpu = & cpu_pool . cpu[current_cpuid];
  thread_t self = cpu -> current_thread;
  DCACHE_INVAL(NULL, CPU_CACHE_ALL);
  
//  DCACHE_FLUSH(&thread, sizeof(thread_t));
//  DCACHE_FLUSH(&queue, sizeof(queue_t*));

  watch (status_t)
  {
    ensure (thread != NULL, DNA_BAD_ARGUMENT);
dna_log(INFO_LEVEL, "(%d) %s -> %s", current_cpuid,
        self -> info . name, thread -> info . name);

    /*
     * Compute the correct times if necessary
     */
    cpu_timer_get (current_cpuid, & current_time);

    DCACHE_INVAL(&cpu->lap_date, sizeof(bigtime_t));
    delta = current_time - cpu -> lap_date;
    DCACHE_FLUSH(&delta,sizeof(bigtime_t));
    self -> info . kernel_time += delta;
    DCACHE_FLUSH(&self->info.kernel_time,sizeof(bigtime_t));
    cpu->prev_thread = self;
    DCACHE_FLUSH(&cpu->prev_thread,sizeof(thread_t));

    /*
     * Update the status of the target thread
     */

    thread -> info . status = DNA_THREAD_RUNNING;
    DCACHE_FLUSH(&thread->info.status,sizeof(thread_status_t));
    thread -> info . cpu_id = current_cpuid;

    /*
     * Save the current context
     */
    DCACHE_INVAL(&self->context, CPU_CONTEXT_SIZE);
    cpu_context_save (& self -> context, & __scheduler_switch_end);
    DCACHE_FLUSH(&self->context, CPU_CONTEXT_SIZE);
    
    /*
     * Check if self is IDLE. In this case, remove CPU
     * from the available list. If target is IDLE, restore
     * the processor status to READY.
     */
    if (self == cpu -> idle_thread)
    {
dna_log(VERBOSE_LEVEL, "CPU(%d) << RUNNING", cpu -> id);
      
      lock_acquire (& cpu_pool . queue . lock);
      queue_extract (& cpu_pool . queue, cpu);
      lock_release (& cpu_pool . queue . lock);

      cpu -> status = DNA_CPU_RUNNING;
    }
    else if (thread == cpu -> idle_thread)
    {
dna_log(VERBOSE_LEVEL, "CPU(%d) >> READY", cpu -> id);

      cpu -> status = DNA_CPU_READY;

      lock_acquire (& cpu_pool . queue . lock);
      queue_add (& cpu_pool . queue, cpu);
      lock_release (& cpu_pool . queue . lock);
    }

    /*
     * Update the processor's status
     */

    cpu -> lap_date = current_time;
    DCACHE_FLUSH(&cpu->lap_date, sizeof(bigtime_t));
    cpu -> current_thread = thread;
    DCACHE_FLUSH(&cpu->current_thread, sizeof(thread_t));
    /*
     * Release the queue's lock if queue is not NULL.
     */

    if (queue != NULL)
    {
      lock_release (& queue -> lock);
    }

    /*/
     * Load the target context
     */
    DCACHE_INVAL(&thread->context, CPU_CONTEXT_SIZE);

    cpu_context_load (& thread -> context);
    DCACHE_FLUSH(&thread->context, CPU_CONTEXT_SIZE);
   
    /*
     * FIXME: Find a better idea for what follows 
     */

    __asm__ volatile ("__scheduler_switch_end:");

    cpu = & cpu_pool . cpu[cpu_mp_id()];
    /*
     * prev and current threads are necessarily set here
     */
    lock_release (& (cpu-> prev_thread -> lock));
    lock_release (& (cpu-> current_thread -> lock));

    /*
     * Contrary to what I thought at first, we cannot 
     * check if self is running the IDLE thread here, because
     * in the case of the CPU0 (the boot CPU), swithching to IDLE
     * would not branch here but directly in the IDLE thread handler.
     */
    return DNA_OK;
  }
}

/*
 ****/

