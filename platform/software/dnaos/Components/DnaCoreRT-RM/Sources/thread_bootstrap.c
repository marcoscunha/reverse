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

/****f* ThreadPrivate/thread_bootstrap
 * SUMMARY
 * The code required to boostrap a thead.
 *
 * SYNOPSIS
 */

int32_t thread_bootstrap (void * p_signature)

/*
 * ARGUMENTS
 * A thread signature.
 * 
 * RESULT
 * Ignored.
 *
 * SOURCE
 */

{
  int32_t value = 0, current_cpuid;
  thread_t self = NULL, target = NULL;
  bool keep_executing = true;
  thread_signature_t * signature = (thread_signature_t *) p_signature;
  interrupt_status_t it_status = 0;

  /*
   * Get some information about the execution.
   */

  it_status = cpu_trap_mask_and_backup();
  current_cpuid = cpu_mp_id();
  self = cpu_pool . cpu[current_cpuid] . current_thread;
  cpu_trap_restore (it_status);

  /*
   * Execute the thread handler according to its type.
   */

  do
  {
    value = signature -> handler (signature -> arguments);

    keep_executing = (self -> info . type == DNA_RT_THREAD);
    keep_executing = keep_executing && (value == DNA_NO_ERROR);

    if (keep_executing)
    {
      it_status = cpu_trap_mask_and_backup();
      lock_acquire (& self -> lock);

      self -> info . status = DNA_THREAD_SLEEPING;

      scheduler_elect (& target, true);
      scheduler_switch (target, NULL);

      cpu_trap_restore (it_status);
    }
  }
  while (keep_executing);

  thread_exit (value);
  return 0;
}

/*
 ****/

