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
#include <Processor/Cache.h>
void * queue_rem (queue_t * queue)
{
  queue_link_t * item = NULL;
 
  watch (void *)
  {
    DCACHE_INVAL_LOST(&queue->status);
    ensure (queue -> status != 0, NULL);

    DCACHE_INVAL_LOST(&queue->head);
    item = queue -> head;

    queue -> status -= 1;
    DCACHE_FLUSH(&queue->status,sizeof(uint32_t));

    DCACHE_INVAL(&item->next, sizeof(queue_link_t*)); 
    queue -> head = item -> next;
    DCACHE_FLUSH(&queue->head,sizeof(queue_link_t*));

    check (queue_error, queue -> head != NULL ||
        (queue -> head == NULL && queue -> status == 0), NULL);

    item -> next = NULL;
    DCACHE_FLUSH(&item->next, sizeof(queue_link_t*));

    return item;
  }

  rescue (queue_error)
  {
dna_log(PANIC_LEVEL, "Q(0x%x): status %d, item 0x%x",
        queue, queue -> status, item);

    leave;
  }
}

