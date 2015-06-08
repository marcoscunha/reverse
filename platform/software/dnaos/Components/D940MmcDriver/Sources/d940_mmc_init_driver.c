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

#include <Private/MMC.h>
#include <Private/LowLevelOperations.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>
#include <MultiMediaCard/MultiMediaCard.h>

status_t d940_mmc_init_driver (void)
{
  status_t status;
  mmc_callbacks_t callbacks =
  {
    d940_mmc_send_command,
    d940_mmc_read_low,
    d940_mmc_write_low
  };

  watch (status_t)
  {
    dna_memset (& d940_mmc_driver, 0, sizeof (mmc_driver_t));

    /*
     * Try to get the extension.
     */

    status = extension_get ("mmc", (extension_t **) & d940_mmc_driver . mmc);
    ensure (status == DNA_OK, status);

    /*
     * Try to create a card.
     */

    status = d940_mmc_driver . mmc -> create
      (& d940_mmc_driver . card, callbacks);
    check (no_card, status == DNA_OK, status);

    log (VERBOSE_LEVEL, "SD-Card successfully loaded !");

    /*
     * Initialize the interrupts.
     */

    interrupt_attach (0, 9, 0x40, d940_mmc_isr, false);
    return DNA_OK;
  }

  rescue (no_card)
  {
    log (PANIC_LEVEL, "No card inserted, initialization aborted.");
    leave;
  }
}

