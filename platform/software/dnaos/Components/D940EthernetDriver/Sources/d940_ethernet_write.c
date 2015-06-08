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

status_t d940_ethernet_write (void * handler, void * source, int64_t offset,
                                  int32_t * p_count) {
  d940_eth_data_t *   pdata = (d940_eth_data_t *) handler;
  d940_eth_t          d940_ethernet_device = pdata->dev;
  d940_eth_ncr_t      ncr;
  d940_eth_tsr_t      tsr;
  int32_t             buffer_index;
  void *              data = source;
  int32_t             buffer_size;
  int32_t             i;
  int32_t             frame_size = *p_count;
  interrupt_status_t  it_status;

  if(frame_size < 0)
  {
    return DNA_BAD_ARGUMENT;
  }

  if(pdata->tx_write == 0)
  {
    pdata->tx_write = *((int32_t *) data);
    return DNA_OK;
  }

  it_status = cpu_trap_mask_and_backup();
  lock_acquire(&pdata->lock);
   
  /* Check/Clear the status of the transmit */
  cpu_read(UINT32, &(d940_ethernet_device->tsr.raw), tsr.raw);
  cpu_write(UINT32, &(d940_ethernet_device->tsr.raw), tsr.raw);
  
  lock_release(&pdata->lock);
  cpu_trap_restore(it_status);
 
  if(tsr.bits.und)
  {
    log(INFO_LEVEL, "Underrun: Clear the transmit buffer list");
    
    it_status = cpu_trap_mask_and_backup();
    lock_acquire(&pdata->lock);
   
    /* Stop the transmit in case of underrun */
    cpu_read(UINT32, &(d940_ethernet_device->ncr.raw), ncr.raw);
    ncr.bits.te = 0;
    cpu_write(UINT32, &(d940_ethernet_device->ncr.raw), ncr.raw);
    ncr.bits.te = 1;
    cpu_write(UINT32, &(d940_ethernet_device->ncr.raw), ncr.raw);
    
    lock_release(&pdata->lock);
    cpu_trap_restore(it_status);
    
    /* ReInit the transmit */
    pdata->tx_tail = 0;
  
    /* ReInit the semaphore */
    while(semaphore_acquire(pdata->tx_sem, 1, DNA_RELATIVE_TIMEOUT, 0) 
        == DNA_OK);
    semaphore_release(pdata->tx_sem, TX_PACKET_LIMIT, 0);
  
    /* Clear the buffers */
    for(i = 0; i < D940_ETH_TX_BUFFER_COUNT; i++)
    {
      pdata->transmit_descs[i].used = 1;
    }
  }

  buffer_index = pdata->tx_tail;

  while(frame_size > 0)
  {
    cpu_cache_invalidate(CPU_CACHE_DATA, &pdata->transmit_descs[buffer_index],
      sizeof(struct tbde));
    
    buffer_size = D940_ETH_TX_BUFFER_SIZE;
    if(buffer_size > frame_size)
    {
      buffer_size = frame_size;
    }

    /* Copy in the transmit buffer */
    dna_memcpy(&pdata->transmit_buffers[buffer_index * D940_ETH_TX_BUFFER_SIZE],
      data, buffer_size);

    data += buffer_size;
    frame_size -= buffer_size;
    pdata->tx_write -= buffer_size;

    /* Set the transmit buffer as ready to send */
    pdata->transmit_descs[buffer_index].len = buffer_size;
    pdata->transmit_descs[buffer_index].used = 0;

    /* Last chunk ? */
    if(pdata->tx_write != 0)
    {
      pdata->transmit_descs[buffer_index].last = 0;
    }
    else
    {
      pdata->transmit_descs[buffer_index].last = 1;
    }

    buffer_index = NEXT_TX_BUFFER(buffer_index);
  }

  /* Next Packet invalid */
  pdata->transmit_descs[buffer_index].used = 1;
  pdata->tx_tail = buffer_index;

  if(pdata->tx_write == 0)
  {
    /* Force to flush the cache */
    cpu_cache_sync();
  
    it_status = cpu_trap_mask_and_backup();
    lock_acquire(&pdata->lock);
  
    /* Restart the transmission */
    cpu_read(UINT32, &(d940_ethernet_device->ncr.raw), ncr.raw);
    ncr.bits.tstart = 1;
    cpu_write(UINT32, &(d940_ethernet_device->ncr.raw), ncr.raw);
  
    lock_release(&pdata->lock);
    cpu_trap_restore(it_status);

    semaphore_acquire(pdata->tx_sem, 1, 0, 0);
  }
  return DNA_OK;
}

