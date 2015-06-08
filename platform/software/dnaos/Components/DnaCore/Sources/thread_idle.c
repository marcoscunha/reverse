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
#include <stdint.h>

#include <Private/Core.h>
#include <Processor/Processor.h>

/****f* ThreadPrivate/thread_idle
 * SUMMARY
 * The idle thread.
 *
 * SYNOPSIS
 */

int32_t thread_idle (void * data)

/*
 * ARGUMENTS
 * Ignored. Argument data is here to keep GCC happy.
 *
 * RESULT
 * Ignored.
 *
 * SOURCE
 */

{
  while (true)
  {
    cpu_power_wake_on_interrupt ();
  }

  return 0;
}

/*
 ****/

