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

/****f* Port/port_create
 * SUMMARY
 * Create a new port_t.
 *
 * SYNOPSIS
 */

status_t port_create (char * name, int32_t queue_length, int32_t * p_id)

/*
 * ARGUMENTS
 * * name : the name of the port.
 * * queue_length : the length of its queue.
 *
 * RESULT
 * * DNA_NO_MORE_PORT: no more port available
 * * DNA_OUT_OF_MEM: cannot allocate memory to create a port
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  int16_t index;
  port_t port = NULL;
  status_t status;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (name != NULL && p_id != NULL, DNA_BAD_ARGUMENT);
    ensure (queue_length > 0, DNA_BAD_ARGUMENT);

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& port_pool . lock);

    /*
     * Get an empty port slot.
     */

    port = queue_rem (& port_pool . port);
    check (pool_error, port != NULL, DNA_NO_MORE_PORT);

    /*
     * Make the place clean.
     */

    index = port -> id . s . index;
    dna_memset (port, 0, sizeof (struct _port));

    port -> id . s . index = index;
    port -> id . s . value = port_pool . counter;

    semaphore_pool . counter += 1;

    lock_release (& port_pool . lock);
    cpu_trap_restore(it_status);

    /*
     * Creating the messages.
     */

    port -> data = kernel_malloc
      (sizeof (struct _message) * queue_length, true);
    check (no_mem, port -> data != NULL, DNA_OUT_OF_MEM);

    for (int32_t i = 0; i < queue_length; i += 1)
    {
      queue_add (& port -> message, & port -> data[i]);
    }

    /*
     * Creating the semaphores.
     */

    status = semaphore_create (name, queue_length, & port -> write_sem);
    check (wsem_error, status == DNA_OK, status);

    status = semaphore_create (name, 0, & port -> read_sem);
    check (rsem_error, status == DNA_OK, status);

    dna_strcpy (port -> info . name, name);
    port -> info . capacity = queue_length;

    /*
     * Return the port information.
     */

    *p_id = port -> id . raw;
    return DNA_OK;
  }

  rescue (rsem_error)
  {
    semaphore_destroy (port -> write_sem);
  }

  rescue (wsem_error)
  {
    kernel_free (port -> data);
  }

  rescue (no_mem)
  {
    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& port_pool . lock);

    queue_add (& port_pool . port, port);
  }

  rescue (pool_error)
  {
    lock_release (& port_pool . lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

