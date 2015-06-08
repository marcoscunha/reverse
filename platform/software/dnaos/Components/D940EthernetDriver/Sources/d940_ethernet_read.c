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

#include <Private/Ethernet.h>

status_t d940_ethernet_read (void * handler, void * destination,
                                  int64_t offset, int32_t * p_count) {
  d940_eth_data_t *   pdata = (d940_eth_data_t *) handler;
  void *              data = destination;
  int32_t *           return_size = (int32_t *) destination;
  int32_t             data_size = *p_count;
  int32_t             buffer_size;
  int32_t             buffer_index;
  int32_t             frame_size = 0;

  if((*p_count) < 0)
  {
    return DNA_BAD_ARGUMENT;
  }

  /* Wait for a valid receive buffer with a timeout */
  if(semaphore_acquire(pdata->rx_sem, 1, DNA_RELATIVE_TIMEOUT, 
      D940_ETH_READ_TIMEOUT) != DNA_OK)
  {
    *return_size = 0;
    *p_count = 0;
    return DNA_OK;
  }
  
  buffer_index = pdata->rx_tail;

  cpu_cache_invalidate(CPU_CACHE_DATA, &pdata->receive_descs[buffer_index],
      sizeof(struct rbde));
  while(pdata->receive_descs[buffer_index].owner == 1)
  {
    if(pdata->rx_read)
    {
      /* Compute the size of data in the receive buffer */
      buffer_size = D940_ETH_RX_BUFFER_SIZE;
      if(pdata->receive_descs[buffer_index].end == 1)
      {
        buffer_size = pdata->receive_descs[buffer_index].len - frame_size;
      }

      if(data_size >= buffer_size)
      {
        /* Copy in the buffer */
        cpu_cache_invalidate(CPU_CACHE_DATA,
          &pdata->receive_buffers[buffer_index * D940_ETH_RX_BUFFER_SIZE],
          buffer_size);
        dna_memcpy(data, 
          &pdata->receive_buffers[buffer_index * D940_ETH_RX_BUFFER_SIZE],
          buffer_size);
      }   

      /* Set the receive buffer as ready to receive */
      pdata->receive_descs[buffer_index].owner = 0;

      frame_size += buffer_size;
      data += buffer_size;
      data_size -= buffer_size;
    }

    if(pdata->receive_descs[buffer_index].end == 1)
      break;

    buffer_index = NEXT_RX_BUFFER(buffer_index);

    cpu_cache_invalidate(CPU_CACHE_DATA, &pdata->receive_descs[buffer_index],
        sizeof(struct rbde));

    /* Detect a error -> Read the next frame */
    if(pdata->receive_descs[buffer_index].start == 1)
    {
      log(INFO_LEVEL, "Invalid frame");
      frame_size = 0;
      data = destination;
      data_size = *p_count;
    }
  }

  if(pdata->rx_read)
  {
    pdata->rx_read = false;
    pdata->rx_tail = NEXT_RX_BUFFER(buffer_index);
    *p_count = frame_size;
    
    /* Force to flush the cache */
    cpu_cache_sync();
  } else {
    pdata->rx_read = true;

    *return_size =  pdata->receive_descs[buffer_index].len;
    *p_count = sizeof(int32_t);

    /* Put the taken token  */
    semaphore_release(pdata->rx_sem, 1, 0); 
  }
  
  return DNA_OK;
}

