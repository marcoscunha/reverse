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

void queue_insert (queue_t * queue, queue_comparator_t comparator, void * data)
{
  queue_link_t * item = data, * current = NULL, * previous = NULL;

  item -> next = NULL;

  if (queue -> status != 0)
  {
    current = queue -> head;
    while (current != NULL)
    {
      /*
       * The comparator function is <
       */

      if (comparator (current, item))
      {
        if (current == queue -> head)
        {
          queue -> head = item;
          item -> next = current;
        }
        else
        {
          previous -> next = item;
          item -> next = current;
        }

        break;
      }
      else
      {
        previous = current;
        current = current -> next;
      }
    }

    if (current == NULL)
    {
      queue -> tail -> next = item;
      queue -> tail = item;
    }
  }
  else
  {
    queue -> head = item;
    queue -> tail = item;
  }

  queue -> status += 1;
}
