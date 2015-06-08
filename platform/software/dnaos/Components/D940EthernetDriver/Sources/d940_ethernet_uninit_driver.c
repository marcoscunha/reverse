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
#include <MemoryManager/MemoryManager.h>

void  d940_ethernet_uninit_driver (void) {
  d940_eth_data_t * pdata = d940_ethernet_handlers[0];
 
  lock_destroy(&pdata->lock);
  semaphore_destroy(pdata->mio_sem);
  semaphore_destroy(pdata->mio_comp_sem);
  semaphore_destroy(pdata->tx_sem);
  semaphore_destroy(pdata->rx_sem);
  kernel_free(pdata);
}


