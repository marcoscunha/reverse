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
#include <Processor/Processor.h>
#include <Processor/Cache.h>

status_t queue_add (queue_t * queue, void * data)
{
//    DCACHE_INVAL(NULL, CPU_CACHE_ALL);
  queue_link_t * item = data;

#ifdef WRITEBACK  
  if((uintptr_t)&queue->head & (uintptr_t)0x1f){
      dna_printf("queue\n\tqueue->lock %p\n\tqueue->head %p\n\tqueue->tail %p\n", &queue->lock, &queue->head, &queue->tail);
      dna_printf("item \n\titem->next  %p\n", &item->next);
  }
#endif

  watch (status_t)
  {
    DCACHE_INVAL(&item->next, sizeof(void*));
    check (queue_error, item -> next == NULL, DNA_ERROR);

    DCACHE_INVAL(&queue->status, sizeof(uint32_t));
    if (queue -> status == 0)
    {
      queue -> head = item;
      DCACHE_FLUSH(&queue->head, sizeof(queue_link_t*));
      queue -> tail = item;
      DCACHE_FLUSH(&queue->tail, sizeof(queue_link_t*));
    }
    else
    {
      DCACHE_INVAL_LOST(&queue->tail);
      queue -> tail -> next = item;
      DCACHE_FLUSH(&queue->tail->next, sizeof(queue_link_t*));
      queue -> tail = item;
      DCACHE_FLUSH(&queue->tail, sizeof(queue_link_t*));

    }
 
    queue -> status += 1;
    DCACHE_FLUSH(&queue->status, sizeof(uint32_t));

    return DNA_OK;
  }

  rescue (queue_error)
  {
      dna_printf("Q(0x%x): status %d, item 0x%x",
               queue, queue -> status, item);
dna_log(PANIC_LEVEL, "Q(0x%x): status %d, item 0x%x",
        queue, queue -> status, item);

    leave;
  }

}

