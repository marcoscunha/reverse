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

status_t queue_extract (queue_t * queue, void * data)
{
  queue_link_t * item = data;
  queue_link_t * kitem;
  
  DCACHE_INVAL_LOST(&queue->head);

  kitem = queue -> head;

  watch (status_t)
  {
    ensure (queue != NULL && data != NULL, DNA_BAD_ARGUMENT);
    DCACHE_INVAL(&queue->status,sizeof(uint32_t));
    ensure (queue -> status != 0, DNA_ERROR);
 
    if (queue -> head == item)
    {
      queue -> head = item -> next;
      DCACHE_FLUSH(&queue->head, sizeof(queue_link_t*));
    }
    else    
    {
      while (kitem -> next != item && kitem -> next != NULL)
      {
        kitem = kitem -> next;
        DCACHE_FLUSH(&kitem, sizeof(queue_link_t*));
      }

      ensure (kitem -> next == item, DNA_ERROR);

      DCACHE_INVAL(&item->next, sizeof(queue_link_t*));
      kitem -> next = item -> next;
      DCACHE_FLUSH(&kitem->next, sizeof(queue_link_t*));
     
      if (kitem -> next == NULL)
      {
        queue -> tail = kitem;
        DCACHE_FLUSH(&queue->tail, sizeof(queue_link_t*));
      }
    }

    item -> next = NULL;
    DCACHE_FLUSH(&item->next, sizeof(queue_link_t *));
    queue -> status -= 1;
    DCACHE_FLUSH(&queue->status, sizeof(uint32_t));
    
    return DNA_OK;
  }
}

