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
#include <DnaTools/DnaTools.h>

void * queue_lookup (queue_t * queue, queue_inspector_t inspector, ...)
{
  bool result;
  va_list list, list_copy;
  queue_link_t * item = NULL;

  va_start (list, inspector);

  if (queue -> status != 0)
  {
    for (item = queue -> head; item != NULL; item = item -> next)
    {
      result = false;

      va_copy (list_copy, list);
      result = inspector (item, list_copy);
      va_end (list_copy);

      if (result) break;
    }
  }

  va_end (list);
  return item;
}

