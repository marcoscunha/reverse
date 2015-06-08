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
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>
#include <Processor/Processor.h>

driver_t d940_ethernet_module = {
  "d940_ethernet",
  d940_ethernet_init_hardware,
  d940_ethernet_init_driver,
  d940_ethernet_uninit_driver,
  d940_ethernet_publish_devices,
  d940_ethernet_find_device
};

const char * d940_ethernet_devices [] =
{
  "ethernet/d940/0",
  NULL
};

status_t d940_ethernet_init_driver (void)
{
  d940_eth_data_t * pdata = d940_ethernet_handlers[0];
  pdata->ref = 0;
  lock_create(&pdata->lock);
  
  interrupt_attach (0, pdata->it, 0x40, d940_ethernet_isr, false);

  log(INFO_LEVEL, "Driver loaded!");

  return DNA_OK;
}


