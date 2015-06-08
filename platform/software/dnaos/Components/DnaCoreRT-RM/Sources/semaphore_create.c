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

/****f* Semaphore/semaphore_create
 * SUMMARY
 * Create a new semaphore_t.
 *
 * SYNOPSIS
 */

status_t semaphore_create (char * name, int32_t tokens, int32_t * id)

/*
 * ARGUMENTS
 * * name : the name of the semaphore.
 * * tokens : the number of tokens of the semaphore.
 *
 * RESULT
 * * DNA_NO_MORE_SEM: no more semaphore available
 * * DNA_OUT_OF_MEM: cannot allocate memory to create a semaphore
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  int32_t index = 0;
  semaphore_t semaphore = NULL;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (name != NULL && id != NULL, DNA_BAD_ARGUMENT);

    /*
     * Create the semaphore and fill in its information
     */

    semaphore = kernel_malloc (sizeof (struct _semaphore), true);
    ensure (semaphore != NULL, DNA_OUT_OF_MEM);

    /*
     * Fill in the information
     */

    dna_strcpy (semaphore -> info . name, name);
    semaphore -> info . tokens = tokens;

    /*
     * Insert the semaphore if a room is available
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& semaphore_pool . lock);

    for (index = 0; index < DNA_MAX_SEM; index ++)
    {
      if (semaphore_pool . semaphore[index] == NULL)
      {
        semaphore -> id . s . value = semaphore_pool . counter;
        semaphore -> id . s . index = index;

        semaphore_pool . counter += 1;
        semaphore_pool . semaphore[index] = semaphore;

        break;
      }
    }

    lock_release (& semaphore_pool . lock);
    check (pool_error, index < DNA_MAX_SEM, DNA_NO_MORE_SEM);

    cpu_trap_restore(it_status);

    dna_log(INFO_LEVEL, "ID(%d:%d) TOKEN(%d)",
        semaphore -> id . s . value, semaphore -> id . s . index,
        semaphore -> info . tokens);

    *id = semaphore -> id . raw;
    return DNA_OK;
  }

  rescue (pool_error)
  {
    cpu_trap_restore(it_status);
    kernel_free (semaphore);
    leave;
  }
}

/*
 ****/

