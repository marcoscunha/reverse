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

#include <stdbool.h>
#include <Private/Core.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* InterruptPrivate/interrupt_demultiplexer
 * SUMMARY
 * Handler for mulitplexed interrupts.
 *
 * SYNOPSIS
 */

int32_t interrupt_demultiplexer (void * data)

/*
 * ARGUMENTS
 * * data : the ID of the interrupt
 *
 * RESULT
 * * DNA_BAD_ARGUMENT: itn is not a valid interrupt ID
 * * DNA_OK: the operation is successfull
 *
 * SOURCE
 */

{
  queue_link_t * isr;
  status_t status;
  int32_t current_cpuid = cpu_mp_id (), itn = (int32_t) data;
  cpu_t * cpu = & cpu_pool . cpu[current_cpuid];
  queue_t * queue = & cpu -> isr[itn];
  thread_t thread, self = cpu -> current_thread;

  watch (int32_t)
  {
    ensure (itn < CPU_TRAP_COUNT, DNA_BAD_ARGUMENT);

    /*
     * Look for the corresponding handler
     */

    lock_acquire (& queue -> lock);
    status = DNA_UNHANDLED_INTERRUPT;

    for (isr = queue -> head; isr != NULL; isr = isr -> next)
    {
      status = ((isr_t)isr) -> handler (data);

      if (status == DNA_INVOKE_SCHEDULER || status == DNA_HANDLED_INTERRUPT)
      {
        break;
      }
    }

    lock_release (& queue -> lock);

    /*
     * If necessary, invoke the scheduler.
     */

    if (status == DNA_INVOKE_SCHEDULER)
    {
      status = scheduler_elect (& thread, false);
      ensure (status != DNA_ERROR && status != DNA_BAD_ARGUMENT, status);

      if (status != DNA_NO_AVAILABLE_THREAD)
      {
        lock_acquire (& self -> lock);
        self -> info . status = DNA_THREAD_READY;

        if (self != cpu_pool . cpu[cpu_mp_id()] . idle_thread)
        {
          queue = & scheduler . queue[self -> info . affinity];
          lock_acquire (& queue -> lock);
          queue_add (queue, self);
        }

        status = scheduler_switch (thread, queue);
        ensure (status == DNA_OK, status);
      }
    }

    return DNA_OK;
  }
}

/*
 ****/

