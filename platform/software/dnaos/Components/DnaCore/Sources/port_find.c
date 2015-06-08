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

/****f* Port/port_find
 * SUMMARY
 * Finds a port by its name.
 *
 * SYNOPSIS
 */

status_t port_find (char * name, int32_t * p_id)

/*
 * ARGUMENTS
 * * name : port's name
 * * p_id : a placeholder for the port's ID
 *
 * RESULT
 * * DNA_UNKNOWN_PORT: no port correspond to the given name
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  port_t port = NULL;
  int32_t index;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (p_id != NULL, DNA_BAD_ARGUMENT);

    /*
     * Find the requested port.
     */

    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& port_pool . lock);

    for (index = 0; index < DNA_MAX_PORT; index += 1)
    {
      port = & port_pool . data[index];

      if (port != NULL && dna_strcmp (name, port -> info . name) == 0)
      {
        *p_id = port -> id . raw;
        break;
      }
    }

    lock_release (& port_pool . lock);
    cpu_trap_restore (it_status);

    ensure (index != DNA_MAX_PORT, DNA_UNKNOWN_PORT);
    return DNA_OK;
  }
}

/*
 ****/

