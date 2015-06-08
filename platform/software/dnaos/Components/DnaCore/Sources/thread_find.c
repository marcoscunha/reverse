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

/****f* Thread/thread_find
 * SUMMARY
 * Finds a thread by its name.
 *
 * SYNOPSIS
 */

status_t thread_find (char * name, int32_t * tid)

/*
 * ARGUMENTS
 * * name : thread's name
 * * tid : a placeholder for the thread's ID
 *
 * RESULT
 * * DNA_UNKNOWN_THREAD: no thread correspond to the given name
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  thread_t thread = NULL, self = NULL;
  int32_t current_cpuid, index;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (tid != NULL, DNA_BAD_ARGUMENT);

    /*
     * Gather information about the current execution.
     */

    it_status = cpu_trap_mask_and_backup();
    current_cpuid = cpu_mp_id ();
    self = cpu_pool . cpu[current_cpuid] . current_thread;

    /*
     * Find the requested thread. It can be either self,
     * or a thread in self's group.
     */

    if (name == NULL)
    {
      DCACHE_INVAL(&self->id.raw, sizeof(int32_t));
      *tid = self -> id . raw;
      cpu_trap_restore (it_status);
    }
    else
    {
      lock_acquire (& thread_pool . lock);

      for (index = 0; index < DNA_MAX_THREAD; index += 1)
      {
        thread = & thread_pool . data[self -> id . s . group][index];

        if (thread -> id . s . value != 0 &&
            dna_strcmp (name, thread -> info . name) == 0)
        {
          *tid = thread -> id . raw;
          break;
        }
      }

      lock_release (& thread_pool . lock);
      cpu_trap_restore (it_status);

      ensure (index != DNA_MAX_THREAD, DNA_UNKNOWN_THREAD);
    }

    return DNA_OK;
  }
}

/*
 ****/

