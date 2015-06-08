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

/****f* Port/port_get_info
 * SUMMARY
 * Get info from a port.
 *
 * SYNOPSIS
 */

status_t port_get_info (int32_t id, port_info_t * p_info)

/*
 * ARGUMENTS
 * * id : the ID of the port to get_info.
 * * p_info : a pointer to a port_info structure.
 *
 * RESULT
 * * DNA_BAD_PORT_ID: the id parameter is invalid
 * * DNA_BAD_ARGUMENT: the port_info pointer is invalid
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  port_t port = NULL;
  port_id_t pid = { .raw = id };
  interrupt_status_t it_status = 0;
  status_t status = DNA_OK;

  watch (status_t)
  {
    ensure (p_info != NULL, DNA_BAD_ARGUMENT);
    ensure (pid . s . index < DNA_MAX_PORT, DNA_BAD_PORT_ID);

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& port_pool . lock);

    /*
     * Look for the port with ID id
     */

    port = & port_pool . data[pid . s . index];
    check (invalid_port, port != NULL, DNA_BAD_PORT_ID);
    check (invalid_port, port -> id . raw == pid . raw, DNA_BAD_PORT_ID);

    lock_acquire (& port -> lock);
    lock_release (& port_pool . lock);

    /*
     * Copy the port information
     */

    *p_info = port -> info;

    lock_release (& port -> lock);
    cpu_trap_restore(it_status);

    return status;
  }

  rescue (invalid_port)
  {
    lock_release (& port_pool . lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

