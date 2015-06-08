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

#include <Private/DBGU.h>
#include <Private/SYSC.h>
#include <Private/Driver.h>
#include <DnaTools/DnaTools.h>

/*
 * Definition of the driver's module.
 */

driver_t d940_system_module =
{
	"d940_system_driver",
	d940_system_init_hardware,
	d940_system_init_driver,
	d940_system_uninit_driver,
	d940_system_publish_devices,
	d940_system_find_device
};

/*
 * Definition of the internal device drivers.
 */

d940_dbgu_driver_t d940_dbgu_driver;

/*
 * Function init_driver.
 */

status_t d940_system_init_driver (void)
{
  status_t status = DNA_OK;

  watch (status_t)
  {
    /*
     * Initialization of the DBGU driver.
     */

    dna_memset (& d940_dbgu_driver, 0, sizeof (d940_dbgu_driver_t));
    d940_dbgu_driver . port = (d940_dbgu_t) 0xFFFFF200;

    status = semaphore_create ("d940_dbgu_semaphore", 0,
        & d940_dbgu_driver . semaphore);
    ensure (status == DNA_OK, status);

    interrupt_attach (0, 1, 0x40, d940_dbgu_isr, false);

    /*
     * Attachement of the CPU timer's handler.
     */

    interrupt_attach (0, 1, 0x40, d940_timer_isr, false);
    return DNA_OK;
  }
}

