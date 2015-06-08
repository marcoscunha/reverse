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
#include <Processor/IO.h>
#include <MemoryManager/MemoryManager.h>
#include <Platform/PIO.h>

d940_eth_data_t * d940_ethernet_handlers[1];

status_t d940_ethernet_init_hardware (void)
{
  d940_eth_t        d940_ethernet_device = PLATFORM_ETH_BASE;
  d940_pmc_t        d940_pmc_device = PLATFORM_PMC_BASE;
  d940_pio_t        d940_pio_device = PLATFORM_PIOA_BASE;

  d940_eth_ncfgr_t  ncfgr;
  d940_eth_data_t * pdata = NULL;
  uint64_t          mac = 0x119876543210;

  uint32_t          main_freq;
  uint32_t          freq;
  uint32_t          io;
  int32_t           ret_val;
  
  watch (status_t)
  {
    /* /!\ Only for one device /!\ */
    log(INFO_LEVEL, "Loading ethernet driver");
    
    pdata = (d940_eth_data_t *) kernel_malloc(sizeof(d940_eth_data_t), false);
    check (malloc_error, pdata != NULL, DNA_ERROR);
    d940_ethernet_handlers[0] = pdata;
   
    pdata->it = D940_ETH_ID;
    pdata->dev = PLATFORM_ETH_BASE;
    pdata->mio_sem = 0;
    pdata->mio_comp_sem = 0;
    pdata->tx_sem = 0;
    pdata->rx_sem = 0;

    /* Semaphore MIO */
    ret_val = semaphore_create("D940_DRIVER_MIO", 0, &pdata->mio_sem);
    check (sem_error, ret_val == DNA_OK, DNA_ERROR);

    /* Semaphore MIO */
    ret_val = semaphore_create("D940_DRIVER_MIO_COMP", 0, &pdata->mio_comp_sem);
    check (sem_error, ret_val == DNA_OK, DNA_ERROR);
    
    /* Semaphore TX */
    ret_val = semaphore_create("D940_DRIVER_TX", 0, &pdata->tx_sem);
    check (sem_error, ret_val == DNA_OK, DNA_ERROR);

    /* Semaphore RX */
    ret_val = semaphore_create("D940_DRIVER_RX", 0, &pdata->rx_sem);
    check (sem_error, ret_val == DNA_OK, DNA_ERROR);

    /* Enable PIN for Ethernet */ 
    io = 0x00FFE000;
    cpu_write(UINT32, &(d940_pio_device->PDR), io); 
    
    /* Select Port A for Ethernet */
    io = 0x00FFE000;
    cpu_write(UINT32, &(d940_pio_device->ASR), io);
  
    /* PCER -> Clock ETHERNET */
    io = (1 << D940_ETH_ID);
    cpu_write(UINT32, &(d940_pmc_device->PCER), io);
  
    /* Get CPU frequency */
    cpu_read (UINT32, &(d940_pmc_device->MCFR), main_freq);
    freq = ((50 * (((uint32_t)(main_freq & 0x0000FFFF)) << 11)) / 3) >> 1;

    /* Set clock divider */
    cpu_read(UINT32, &(d940_ethernet_device->ncfgr.raw), ncfgr.raw);
    if (freq < 20000000)
      ncfgr.bits.clk = clk_div8;
    else if (freq < 40000000)
      ncfgr.bits.clk = clk_div16; 
    else if (freq < 80000000)
      ncfgr.bits.clk = clk_div32;
    else
      ncfgr.bits.clk = clk_div64;
    cpu_write(UINT32, &(d940_ethernet_device->ncfgr.raw), ncfgr.raw);

    /* Set MAC addr filter */
    cpu_write(UINT32, &(d940_ethernet_device->sa1.addr_l),
        (uint32_t) (mac & 0xFFFFFFFF));
    cpu_write(UINT32, &(d940_ethernet_device->sa1.addr_h),
        (uint32_t) ((mac >> 32) & 0xFFFF));

    /* Set Receive/Transmit buffer descriptor */
    cpu_write(UINT32, &(d940_ethernet_device->rbqp.raw),
        (uint32_t) &(pdata->receive_descs));
    cpu_write(UINT32, &(d940_ethernet_device->tbqp.raw),
        (uint32_t) &(pdata->transmit_descs));

    log(INFO_LEVEL,"Done!");
    return DNA_OK;
  }
  rescue (malloc_error)
  {
    log(INFO_LEVEL, "Failed: no more memory");
    leave;
  }
  rescue(sem_error)
  {
    if(pdata->mio_sem != 0) semaphore_destroy(pdata->mio_sem);
    if(pdata->mio_comp_sem !=0) semaphore_destroy(pdata->mio_comp_sem);
    if(pdata->tx_sem != 0) semaphore_destroy(pdata->tx_sem);
    if(pdata->rx_sem != 0)semaphore_destroy(pdata->rx_sem);
    kernel_free(pdata);
    log(INFO_LEVEL, "Failed: semaphore creation error");
    leave;
  }
}

