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
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* Port/port_read
 * SUMMARY
 * Read a message from a port.
 *
 * SYNOPSIS
 */

status_t port_read (int32_t id, int32_t * p_code, void * buffer,
    int32_t size, int32_t flags, bigtime_t timeout)

/*
 * ARGUMENTS
 * * id : the ID of the port to acquire.
 * * p_code : pointer to the message code.
 * * buffer : pointer to the message buffer.
 * * size : message buffer's size.
 * * flags : operation flags.
 * * timeout : potential timeout.
 *
 * RESULT
 * * DNA_BAD_PORT_ID if the pid parameter is invalid.
 * * DNA_OK if the operation succeded.
 *
 * SOURCE
 */

{
  int32_t read_sem, write_sem;
#if 0
  int32_t data_size = 0;
#endif
  port_t port = NULL;
  port_id_t pid = { .raw = id };
  message_t message = NULL;
  status_t status;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (pid . s . index < DNA_MAX_PORT, DNA_BAD_PORT_ID);

    /*
     * Look for the port with ID pid.
     */

    it_status = cpu_trap_mask_and_backup ();
    lock_acquire (& port_pool . lock);

    port = & port_pool . data[pid . s . index];
    check (bad_portid, port != NULL, DNA_BAD_PORT_ID);
    check (bad_portid, port -> id . raw == pid . raw, DNA_BAD_PORT_ID);

    lock_acquire (& port -> lock);
    lock_release (& port_pool . lock);

    check (bad_port, ! port -> closed ||
        port -> mailbox . status != 0, DNA_ERROR);

    read_sem = port -> read_sem;
    write_sem = port -> write_sem;

    lock_release (& port -> lock);
    cpu_trap_restore(it_status);

    /*
     * Acquire the semaphore.
     */

    status = semaphore_acquire (read_sem, 1, flags, timeout);
    ensure (status == DNA_OK, status);

    /*
     * Get the port once again, just to make sure
     * it has not been destroyed in the meantime.
     */

    it_status = cpu_trap_mask_and_backup ();
    lock_acquire (& port_pool . lock);

    port = & port_pool . data[pid . s . index];
    check (bad_portid, port != NULL, DNA_BAD_PORT_ID);
    check (bad_portid, port -> id . raw == pid . raw, DNA_BAD_PORT_ID);

    lock_acquire (& port -> lock);
    lock_release (& port_pool . lock);

    check (bad_port, ! port -> closed ||
        port -> mailbox . status != 0, DNA_ERROR);

    /*
     * Get the message from the mailbox, and
     * put it back in the message queue.
     */

    message = queue_rem (& port -> mailbox);
    check (bad_port, message != NULL, DNA_ERROR);

    *p_code = message -> code;
#if 0
    data_size = size >= message -> size ? message -> size : size;
#endif
    dna_memcpy (buffer, message -> buffer, size);

    queue_add (& port -> message, port);

    lock_release (& port -> lock);
    cpu_trap_restore(it_status);

    /*
     * Delete the message, release the write semaphore, and return.
     */

    status = semaphore_release (write_sem, 1, DNA_NO_RESCHEDULE);
    ensure (status == DNA_OK, status);

    return DNA_OK;
  }

  rescue (bad_port)
  {
    lock_release (& port -> lock);
    cpu_trap_restore(it_status);
    leave;
  }

  rescue (bad_portid)
  {
    lock_release (& port_pool . lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

