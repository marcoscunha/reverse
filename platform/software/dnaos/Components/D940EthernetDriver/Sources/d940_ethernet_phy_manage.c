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

void d940_ethernet_phy_manage(d940_eth_data_t *pdata)
{
  d940_eth_t            d940_ethernet_device;
  d940_eth_ncfgr_t      ncfgr;
  int32_t               i;
  uint32_t              status = 0;
  interrupt_status_t    it_status;

  d940_ethernet_device = pdata->dev;

  if(pdata->phy_id != -1)
  {
    for(i = 0; i < D940_ETH_PHY_STATUS_RETRY; i++)
    {
      if((status = d940_ethernet_mdio_read(pdata, MII_BMSR)) 
          & BMSR_LSTATUS)
        break;
    }
  }
  else
  {
    /* Fake state */
    status = BMSR_100FULL | BMSR_LSTATUS;
  }

  status &= BMSR_100FULL | BMSR_100HALF | BMSR_10FULL 
            | BMSR_10HALF | BMSR_LSTATUS;

  /* No change ? */
  if(status == pdata->phy_status)
    return;

  pdata->phy_status = status;

  it_status = cpu_trap_mask_and_backup();
  lock_acquire(&pdata->lock);

  /* Configure the PHY */
  cpu_read(UINT32, &(d940_ethernet_device->ncfgr.raw), ncfgr.raw);
  ncfgr.bits.spd = ((status & BMSR_100FULL)
                    || (status & BMSR_100HALF))? speed_100mb : speed_10mb;
  ncfgr.bits.fd = ((status & BMSR_100FULL)
                    || (status & BMSR_10FULL))? 1 : 0;
  cpu_write(UINT32, &(d940_ethernet_device->ncfgr.raw), ncfgr.raw);

  lock_release(&pdata->lock);
  cpu_trap_restore(it_status);

  /* Show the status */
  if(status & BMSR_LSTATUS) {
    log(INFO_LEVEL, "Link: up");
  } else {
    log(INFO_LEVEL, "Link: down");
  }

  /* Show connexion info */
  if(ncfgr.bits.spd == speed_100mb) {
    log(INFO_LEVEL, "Speed: 100mb");
  } else {
    log(INFO_LEVEL, "Speed: 10mb");
  }
  if(ncfgr.bits.fd == 1) {
    log(INFO_LEVEL, "Full-duplex: yes");
  } else {
    log(INFO_LEVEL, "Full-duplex: no");
  }
}

