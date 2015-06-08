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

/****f* Thread/thread_exit
 * SUMMARY
 * Exit from a thread.
 *
 * SYNOPSIS
 */

void thread_exit (int32_t value)

/*
 * ARGUMENTS
 * * value : the thread's return value
 *
 * SOURCE
 */

{
  uint32_t current_cpuid = 0;
  thread_t self = NULL;
  thread_t target = NULL, p = NULL;
#if 0
  interrupt_status_t it_status = 0;
#endif

  /*
   * First, we lock ourselves
   */

#if 0
  it_status = cpu_trap_mask_and_backup();
#endif
  cpu_trap_mask_and_backup();

  current_cpuid = cpu_mp_id();
  self = cpu_pool . cpu[current_cpuid] . current_thread;

  /*
   * And we place the return value in our structure
   */

  lock_acquire (& self -> lock);
  self -> signature . return_value = value;

  /*
   * Mark self as ended.
   */

  self -> info . status = DNA_THREAD_ENDED;
  DCACHE_FLUSH(&self -> info . status, sizeof(thread_status_t));

  lock_acquire (& self -> wait . lock);
  lock_release (& self -> lock);

  /*
   * Then we can wake up the waiting threads
   */

  while ((p = queue_rem (& self -> wait)) != NULL)
  {
    lock_acquire (& p -> lock);
 
    p -> resource_queue = NULL;
    DCACHE_FLUSH(&p->resource_queue, sizeof(queue_t*));
    p -> info . status = DNA_THREAD_READY;
    DCACHE_FLUSH(&p->info.status, sizeof(thread_status_t));
    p -> info . resource = DNA_NO_RESOURCE;
    DCACHE_FLUSH(&p->info.resource, sizeof(thread_resource_t));
    dna_log(VERBOSE_LEVEL, " %s  DNA_NO_RESOURCE ", p-> info . name);

    p -> info . resource_id = -1;
    DCACHE_FLUSH(&p->info.resource_id, sizeof(int32_t));


    scheduler_dispatch (p);
  }

  lock_release (& self -> wait . lock);

  /*
   * Elect a the next thread and run it
   */

  scheduler_elect (& target, true);
  scheduler_switch (target, NULL);
}

/*
 ****/

