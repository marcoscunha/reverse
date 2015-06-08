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
#include <Processor/Processor.h>

status_t d940_ethernet_open (char * name, int32_t mode, void ** data)
{
  d940_eth_t            d940_ethernet_device;
  d940_eth_data_t *     pdata;
  d940_eth_int_t        interrupt;
  d940_eth_ncr_t        ncr;
  d940_eth_ncfgr_t      ncfgr;
  d940_eth_usrio_t      usrio;
  interrupt_status_t    it_status;
  uint32_t              i;
 
  if(data == NULL) return DNA_ERROR;

  /* /!\ Only for one device /!\ */
  if(dna_strcmp (name, d940_ethernet_devices[0]) != 0) return DNA_ERROR;
  pdata = d940_ethernet_handlers[0];

  /* Exclusive access */
  it_status = cpu_trap_mask_and_backup();
  lock_acquire(&pdata->lock);
  if(pdata->ref != 0) 
  {
    lock_release(&pdata->lock);
    cpu_trap_restore(it_status);
    return DNA_ERROR;
  }
  pdata->ref += 1;
  lock_release(&pdata->lock);
  cpu_trap_restore(it_status);

  /* Set device access */
  d940_ethernet_device = pdata->dev;
  *data = pdata;
  
  /* Init all the semaphore */
  while(semaphore_acquire(pdata->mio_sem, 1, DNA_RELATIVE_TIMEOUT, 0) 
        == DNA_OK);
  while(semaphore_acquire(pdata->mio_comp_sem, 1, DNA_RELATIVE_TIMEOUT, 0) 
        == DNA_OK);
  while(semaphore_acquire(pdata->tx_sem, 1, DNA_RELATIVE_TIMEOUT, 0) 
        == DNA_OK);
  while(semaphore_acquire(pdata->rx_sem, 1, DNA_RELATIVE_TIMEOUT, 0) 
        == DNA_OK);

  /* Set the limit of packet in the transmit buffers */
  semaphore_release(pdata->mio_sem, 1, 0);
  semaphore_release(pdata->tx_sem, TX_PACKET_LIMIT, 0);
  
  /* Initialisation of transmit buffer descriptor */
  for(i = 0; i < D940_ETH_TX_BUFFER_COUNT; i++)
  {
    pdata->transmit_descs[i].addr = 
      ((uint32_t)(& pdata->transmit_buffers[i * D940_ETH_TX_BUFFER_SIZE]));
    
    pdata->transmit_descs[i].used = 1;
    pdata->transmit_descs[i].wrap = 0;
    pdata->transmit_descs[i].no_crc = 0;
  }
  pdata->transmit_descs[i-1].wrap = 1;

  /* Initialisation of receive buffer descriptor */
  for(i = 0; i < D940_ETH_RX_BUFFER_COUNT; i++)
  {
    pdata->receive_descs[i].addr = 
      ((uint32_t)(& pdata->receive_buffers[i * D940_ETH_RX_BUFFER_SIZE])) >> 2;
    
    pdata->receive_descs[i].owner = 0;
    pdata->receive_descs[i].wrap = 0;
  }
  pdata->receive_descs[i-1].wrap = 1;
  
  /* Initialisation of the tails */
  pdata->tx_tail = 0;
  pdata->rx_tail = 0;
  pdata->rx_read = false;
  pdata->tx_write = 0;

  /* Initialisation of the stats */
  pdata->tx_count = 0;
  pdata->rx_count = 0;

  /* Configure the PHY */
  pdata->phy_status = 0;
  d940_ethernet_phy_probe(pdata);
  d940_ethernet_phy_manage(pdata);
 
  it_status = cpu_trap_mask_and_backup();
  lock_acquire(&pdata->lock);
  
  /* Configure the device (depends of PHY) */
  cpu_read(UINT32, &(d940_ethernet_device->ncfgr.raw), ncfgr.raw);
  ncfgr.bits.rbof = 0;
  ncfgr.bits.drfcs = 1;
  ncfgr.bits.pae = 1;
  cpu_write(UINT32, &(d940_ethernet_device->ncfgr.raw), ncfgr.raw);

  /* Configure MII */
  cpu_read(UINT32, &(d940_ethernet_device->usrio.raw), usrio.raw);
  usrio.bits.clken = 0;
  usrio.bits.rmii = 0;
  cpu_write(UINT32, &(d940_ethernet_device->usrio.raw), usrio.raw);

  /* Enable Interrupt (NO TXUBR & NO MFD) */
  interrupt.raw = 0x00003cf6;
  cpu_write(UINT32, &(d940_ethernet_device->ier.raw), interrupt.raw);

  /* Start Receive/Transmit */
  cpu_read(UINT32, &(d940_ethernet_device->ncr.raw), ncr.raw);
  ncr.bits.re = 1;
  ncr.bits.te = 1;
  ncr.bits.clrstat = 1;
  cpu_write(UINT32, &(d940_ethernet_device->ncr.raw), ncr.raw);
  
  lock_release(&pdata->lock);
  cpu_trap_restore(it_status);
 
  return DNA_OK;  
}

