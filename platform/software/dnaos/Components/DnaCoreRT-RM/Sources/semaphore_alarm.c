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

/****f* SemaphorePrivate/semaphore_alarm
 * SUMMARY
 * Semaqphore acquire alarm.
 *
 * SYNOPSIS
 */

status_t semaphore_alarm (void * data)

/*
 * ARGUMENTS
 * * data : a thread_t element.
 *
 * RETURN
 * * DNA_BAD_ARGUMENT: the thread parameter is not valid
 * * DNA_ERROR: the thread is not a valid thread
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  thread_t thread = data;
  status_t status = DNA_OK;

  watch (status_t)
  {
    ensure (thread != NULL, DNA_BAD_ARGUMENT);
    ensure (thread -> resource_queue != NULL, DNA_ERROR);
    ensure (thread -> info . status == DNA_THREAD_WAITING, DNA_ERROR);
    ensure (thread -> info . resource == DNA_RESOURCE_SEMAPHORE, DNA_ERROR);

    /*
     * Lock the thread's resource queue.
     * Extract the thread from the waiting list.
     */

    lock_acquire (& thread -> resource_queue -> lock);
    status = queue_extract (thread -> resource_queue, thread);
    lock_release (& thread -> resource_queue -> lock);

    /*
     * If the thread was waiting, we can dispatch it
     */

    if (status == DNA_OK)
    {
      lock_acquire (& thread -> lock);

      thread -> resource_queue = NULL;
      thread -> info . sem_tokens = 0;
      thread -> info . resource = DNA_NO_RESOURCE;
      thread -> info . resource_id = -1;

      thread -> info . status = DNA_THREAD_READY;
      status = scheduler_dispatch (thread);
    }

    return status;
  }
}

/*
 ****/

