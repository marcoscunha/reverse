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

#include <Private/KickStart.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* kickstart/dna_kickstart
 * SUMMARY
 * Start the DNA operating system.
 *
 * SYNOPSIS
 */

status_t system_kickstart (void)

/*
 * FUNCTION
 *
 * SOURCE
 */

{
  watch (status_t)
  {
    if (cpu_mp_id() == 0)
    {
      status_t status = DNA_OK;

      /*
       * Create the components
       */

      status = memory_component . create ();
      ensure (status == DNA_OK, status);

      status = core_component . create ();
      ensure (status == DNA_OK, status);

      status = vfs_component . create ();
      ensure (status == DNA_OK, status);

      /*
       * Start the components. We need the processors to
       * be in IDLE state to proceed correclty. TODO we should
       * probably find a way to wait ...
       */

      dna_log(INFO_LEVEL, "Starting DNA Operating System");
      cpu_mp_proceed ();

      status = memory_component . start ();
      ensure (status == DNA_OK, status);

      status = vfs_component . start ();
      ensure (status == DNA_OK, status);
    }
    else cpu_mp_wait();

    return core_component . start ();
  }
}

/*
 ****/

