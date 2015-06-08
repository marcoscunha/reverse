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

/****f* Semaphore/semaphore_get_info
 * SUMMARY
 * Get info from a semaphore.
 *
 * SYNOPSIS
 */

status_t semaphore_get_info (int32_t id, semaphore_info_t * info)

/*
 * ARGUMENTS
 * * id : the ID of the semaphore to get_info.
 * * sem_info : a pointer to a sem_info structure.
 *
 * RESULT
 * * DNA_BAD_SEM_ID: the id parameter is invalid
 * * DNA_BAD_ARGUMENT: the sem_info pointer is invalid
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  semaphore_t sem = NULL;
  semaphore_id_t sid = { .raw = id };
  interrupt_status_t it_status = 0;
  status_t status = DNA_OK;

  watch (status_t)
  {
    ensure (info != NULL, DNA_BAD_ARGUMENT);
    ensure (sid . s . index < DNA_MAX_SEM, DNA_BAD_SEM_ID);

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& semaphore_pool . lock);

    /*
     * Look for the semaphore with ID id
     */

    sem = & semaphore_pool . data[sid . s . index];
    check (invalid_semaphore, sem != NULL, DNA_BAD_SEM_ID);
    DCACHE_INVAL(&sem->id.raw, sizeof(int32_t));
    check (invalid_semaphore, sem -> id . raw == sid . raw, DNA_BAD_SEM_ID);

    lock_acquire (& sem -> lock);
    lock_release (& semaphore_pool . lock);

    /*
     * Copy the semaphore information
     */
//    cpu_dcache_inv_addr(&sem -> info.tokens);

dna_log(VERBOSE_LEVEL, "ID(%d:%d) TOKEN(%d)",
        sem -> id . s . value, sem -> id . s . index, sem -> info . tokens);

    DCACHE_INVAL(&sem->info, sizeof(port_info_t));
    *info = sem -> info;
    DCACHE_FLUSH(info, sizeof(port_info_t));

    lock_release (& sem -> lock);
    cpu_trap_restore(it_status);

    return status;
  }

  rescue (invalid_semaphore)
  {
    lock_release (& semaphore_pool . lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

