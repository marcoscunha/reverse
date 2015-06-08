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

uint16_t d940_ethernet_mdio_read(d940_eth_data_t *pdata, int reg_id)
{
  d940_eth_t          d940_ethernet_device = pdata->dev;
  d940_eth_man_t      man;
  uint16_t            value;
  interrupt_status_t  it_status;

  semaphore_acquire(pdata->mio_sem, 1, 0, 0);

  d940_ethernet_mdio_enable(pdata);

  man.bits.sof = MAN_SOF;
  man.bits.rw = MAN_READ;
  man.bits.phya = pdata->phy_id;
  man.bits.rega = reg_id;
  man.bits.code = MAN_CODE;
  
  it_status = cpu_trap_mask_and_backup();
  lock_acquire(&pdata->lock);
  
  /* Send the command to the PHY */
  cpu_write(UINT32, &(d940_ethernet_device->man.raw), man.raw);

  lock_release(&pdata->lock);
  cpu_trap_restore(it_status);

  /* Wait the reply of the PHY */
  semaphore_acquire(pdata->mio_comp_sem, 1, 0, 0);
  
  it_status = cpu_trap_mask_and_backup();
  lock_acquire(&pdata->lock);
  
  /* Read the value returned by the PHY */
  cpu_read(UINT32, &(d940_ethernet_device->man.raw), man.raw);
  
  lock_release(&pdata->lock);
  cpu_trap_restore(it_status);
  
  value = man.bits.data;
  d940_ethernet_mdio_disable(pdata);

  semaphore_release(pdata->mio_sem, 1, 0);

  return value;
}

