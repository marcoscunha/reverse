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

bool d940_ethernet_phy_probe(d940_eth_data_t *pdata)
{
   uint16_t phyid1, phyid2;

   for (pdata->phy_id = 0; pdata->phy_id < 32; pdata->phy_id++) {
      phyid1 = d940_ethernet_mdio_read(pdata, MII_PHYSID1);
      phyid2 = d940_ethernet_mdio_read(pdata, MII_PHYSID2);

      if (phyid1 != 0xffff && phyid1 != 0x0000
          && phyid2 != 0xffff && phyid2 != 0x0000)
         break;
   }

  if (pdata->phy_id == 32)
  {
    pdata->phy_id = -1;
    log(INFO_LEVEL, "No PHY detected");
    return false;
  }

  log(INFO_LEVEL, "Detected PHY at address %d (ID %x:%x)", pdata->phy_id, phyid1, phyid2);
  return true;
}

