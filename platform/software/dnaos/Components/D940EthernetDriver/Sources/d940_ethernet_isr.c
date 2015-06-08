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
#include <DnaTools/Log.h>

int32_t d940_ethernet_isr (void * data)
{
  d940_eth_data_t * pdata = d940_ethernet_handlers[0];
  d940_eth_t        d940_ethernet_device = pdata->dev;
  d940_eth_int_t    isr;
  uint32_t          r_packet;
  uint32_t          t_packet;
  int32_t           status =  DNA_HANDLED_INTERRUPT;
  int32_t           itn = (int32_t)data;

  if(itn != pdata->it)
    return DNA_UNHANDLED_INTERRUPT;

  cpu_read(UINT32, (&d940_ethernet_device->isr.raw), isr.raw);
  cpu_read(UINT32, (&d940_ethernet_device->fro), r_packet);
  cpu_read(UINT32, (&d940_ethernet_device->fto), t_packet);

  if(isr.bits.mfd)
  {
    semaphore_release(pdata->mio_comp_sem, 1, 0);
  }
  if(isr.bits.rxubr)
  {
    log(INFO_LEVEL, "Ethernet: RXUBR it");
  }
  if(isr.bits.txubr)
  {
    log(INFO_LEVEL, "Ethernet: TXUBR it");
  }
  if(isr.bits.tund)
  {
    log(INFO_LEVEL, "Ethernet: TUND it");
  }
  if(isr.bits.rle)
  {
    log(INFO_LEVEL, "Ethernet: RLE it");
  }
  if(isr.bits.txerr)
  {
    log(INFO_LEVEL, "Ethernet: TXERR it");
  }
  if(isr.bits.rovr)
  {
    log(INFO_LEVEL, "Ethernet: ROVR it");
  }
  if(isr.bits.hresp)
  {
    log(INFO_LEVEL, "Ethernet: HRESP it")
  }
  if(isr.bits.pfr)
  {
    log(INFO_LEVEL, "Ethernet: PFR it")
  }
  if(isr.bits.ptz)
  {
    log(INFO_LEVEL, "Ethernet: PTZ it")
  }
  if(isr.bits.tcomp)
  {
    semaphore_release(pdata->tx_sem, t_packet, DNA_NO_RESCHEDULE);
    status = DNA_INVOKE_SCHEDULER;
  }
  if(isr.bits.rcomp)
  {
    semaphore_release(pdata->rx_sem, r_packet, DNA_NO_RESCHEDULE);
    status = DNA_INVOKE_SCHEDULER;
  }
  
  pdata->rx_count += r_packet;
  pdata->tx_count += t_packet; 

  return status;
}

