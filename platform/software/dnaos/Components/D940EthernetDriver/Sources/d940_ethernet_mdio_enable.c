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

void d940_ethernet_mdio_enable(d940_eth_data_t *pdata)
{
  d940_eth_t          d940_ethernet_device = pdata->dev;
  d940_eth_ncr_t      ncr;
  d940_eth_int_t      interrupt;
  interrupt_status_t  it_status;

  it_status = cpu_trap_mask_and_backup();
  lock_acquire(&pdata->lock);

  /* Enable PHY management */
  cpu_read(UINT32, &(d940_ethernet_device->ncr.raw), ncr.raw);
  ncr.bits.mpe = 1;
  cpu_write(UINT32, &(d940_ethernet_device->ncr.raw), ncr.raw);
 
  /* Enable PHY management interruption */
  interrupt.raw = 0;
  interrupt.bits.mfd = 1;
  cpu_write(UINT32, &(d940_ethernet_device->ier.raw), interrupt.raw);

  lock_release(&pdata->lock);
  cpu_trap_restore(it_status);
}
