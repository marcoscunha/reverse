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

#include <DnaTools/DnaTools.h>
#include <Private/Core.h>

bool scheduler_comparator (void * item1, void * item2)
{
  thread_t thread1 = item1;
  thread_t thread2 = item2;
  
  watch (bool)
  {
    ensure (thread1 != NULL && thread2 != NULL, false);
    return thread1 -> priority > thread2 -> priority;
  }
}

/*
 ****/

