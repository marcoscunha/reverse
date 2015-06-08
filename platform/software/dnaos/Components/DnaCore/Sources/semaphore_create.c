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
  int16_t index;
  semaphore_t semaphore = NULL;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (name != NULL && id != NULL, DNA_BAD_ARGUMENT);

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& semaphore_pool . lock);

    /*
     * Get an empty semaphore slot.
     */

    semaphore = queue_rem (& semaphore_pool . semaphore);
    check (pool_error, semaphore != NULL, DNA_NO_MORE_SEM);
   
    /*
     * Make the place clean.
     */

    index = semaphore -> id . s . index;
    dna_memset (semaphore, 0, sizeof (struct _semaphore));

    semaphore -> id . s . index = index;
    semaphore -> id . s . value = semaphore_pool . counter;
    DCACHE_FLUSH(&semaphore->id,sizeof(semaphore_id_t));

    semaphore_pool . counter += 1;
    DCACHE_FLUSH(&semaphore_pool.counter,sizeof(uint16_t));

    /*
     * Release the pool.
     */

    lock_release (& semaphore_pool . lock);
    cpu_trap_restore(it_status);

    /*
     * Fill in the information.
     */

    dna_strcpy (semaphore -> info . name, name);
    DCACHE_FLUSH(&semaphore->info.name,sizeof(char)*DNA_NAME_LENGTH);
    semaphore -> info . tokens = tokens;
    DCACHE_FLUSH(&semaphore->info.tokens,sizeof(semaphore_id_t));

    /*
     * Return the semaphore ID.
     */

dna_log(VERBOSE_LEVEL, "ID(%d:%d) TOKEN(%d)",
        semaphore -> id . s . value, semaphore -> id . s . index,
        semaphore -> info . tokens);

    *id = semaphore -> id . raw;    
    DCACHE_FLUSH(id,sizeof(int32_t));
    return DNA_OK;
  }

  rescue (pool_error)
  {
    cpu_trap_restore(it_status);
    lock_release (& semaphore_pool . lock);
    leave;
  }
}

/*
 ****/

