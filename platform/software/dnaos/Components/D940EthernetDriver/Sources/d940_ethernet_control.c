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
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

status_t d940_ethernet_control (void * handler, int32_t function,
    void * arguments, int32_t * p_ret)
{
  d940_eth_data_t *   pdata = (d940_eth_data_t *) handler;
  d940_eth_t          d940_ethernet_device = pdata->dev;
  switch (function)
  {
    case DNA_GET_ETH_LINK_STATUS:
      d940_ethernet_phy_manage(pdata);
      *(((uint32_t **)arguments)[0]) = pdata->phy_status & BMSR_LSTATUS;
      break;
    case DNA_GET_ETH_MAC:
    {
        interrupt_status_t  it_status;
        uint32_t            tmp, tmp2;
  
        it_status = cpu_trap_mask_and_backup();
        lock_acquire(&pdata->lock);

        /* Copy the MAC address */
        cpu_read(UINT32, &(d940_ethernet_device->sa1.addr_l), tmp);
        cpu_read(UINT32, &(d940_ethernet_device->sa1.addr_h), tmp2);
  
        lock_release(&pdata->lock);
        cpu_trap_restore(it_status);

        dna_memcpy(((uint32_t **)arguments)[0], &tmp, 4);
        dna_memcpy(((uint32_t **)arguments)[0] +1, &tmp2, 2);

      break;
    }
    case DNA_GET_ETH_RX_STATS:
      dna_memcpy(((uint32_t **)arguments)[0], &pdata->rx_count, 
                  sizeof(int32_t));
      break;
    case DNA_GET_ETH_TX_STATS:
      dna_memcpy(((uint32_t **)arguments)[0], &pdata->tx_count, 
                  sizeof(int32_t));
      break;
    default:
      return DNA_ERROR;
  }

  return DNA_OK;
}
