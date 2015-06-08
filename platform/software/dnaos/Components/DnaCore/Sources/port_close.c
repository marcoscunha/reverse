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
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* Port/port_close
 * SUMMARY
 * Close a port.
 *
 * SYNOPSIS
 */

status_t port_close (int32_t id)

/*
 * ARGUMENTS
 * * id : the ID of the port to acquire.
 *
 * RESULT
 * * DNA_BAD_PORT_ID if the pid parameter is invalid.
 * * DNA_OK if the operation succeded.
 *
 * SOURCE
 */

{
  port_t port = NULL;
  port_id_t pid = { .raw = id };
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (pid . s . index < DNA_MAX_PORT, DNA_BAD_PORT_ID);

    /*
     * Look for the port with ID pid.
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& port_pool . lock);

    port = & port_pool . data[pid . s . index];
    check (bad_portid, port != NULL, DNA_BAD_PORT_ID);
    check (bad_portid, port -> id . raw == pid . raw, DNA_BAD_PORT_ID);

    lock_acquire (& port -> lock);
    lock_release (& port_pool . lock);

    /*
     * Close the port and return.
     */

    port -> closed = true;

    lock_release (& port -> lock);
    cpu_trap_restore(it_status);

    return DNA_OK;
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

