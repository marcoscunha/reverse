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

/****t* Thread/thread_yield
 * SUMMARY
 * Yield some CPU time to someone else.
 *
 * SYNOPSIS
 */

status_t thread_yield (void)

/*
 * RESULT
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  status_t status = DNA_OK;
  thread_t thread = NULL, self = NULL;
  queue_t * queue = NULL;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    it_status = cpu_trap_mask_and_backup();
    self = cpu_pool . cpu[cpu_mp_id()] . current_thread;

    status = scheduler_elect (& thread, false);
    ensure (status != DNA_ERROR && status != DNA_BAD_ARGUMENT, status);

    if (status != DNA_NO_AVAILABLE_THREAD)
    {
      lock_acquire (& self -> lock);
      self -> info . status = DNA_THREAD_READY;
      queue = & scheduler . queue[self -> info . affinity];

      lock_acquire (& queue -> lock);
      dna_printf("0x%08x queue->lock %d %s\n",&queue->lock, queue->lock, __func__);
      queue_add (queue, self);

      status = scheduler_switch (thread, queue);
      ensure (status == DNA_OK, status);
    }

    cpu_trap_restore(it_status);
    return status;
  }
}

/*
 ****/

