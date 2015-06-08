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

#include <Private/Driver.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>
#include <Platform/MCI.h>

status_t d940_mmc_init_hardware (void)
{
  uint32_t mode = 0, clock_divider, plla_mula, plla_diva;
  uint32_t master_clock, plla_config, main_freq, dtoconfig = 0x70;

  /*
   * Get timing informations.
   */

	cpu_read (UINT32, & (PLATFORM_PMC_BASE -> MCFR), main_freq);
	main_freq = ((uint32_t)(main_freq & 0x0000FFFF)) << 11;

	cpu_read (UINT32, & (PLATFORM_PMC_BASE -> PLLAR), plla_config);
	plla_mula = ((uint32_t)(plla_config & 0x07FF0000)) >> 16;
	plla_diva = plla_config & 0x0000000F;

	master_clock = ((plla_mula * main_freq) / plla_diva) >> 1;

  /*
   * Set PIOC mask for the MCI controller.
   */

	cpu_write (UINT32, & (PLATFORM_PIOC_BASE -> PUER), 0x0F800000UL);
	cpu_write (UINT32, & (PLATFORM_PIOC_BASE -> PUDR), 0x00400000UL);
	cpu_write (UINT32, & (PLATFORM_PIOC_BASE -> ASR), 0x0FC00000UL);
	cpu_write (UINT32, & (PLATFORM_PIOC_BASE -> PDR), 0x0FC00000UL);

  /*
   * Enable the MCI clock on the PMC.
   */

	cpu_write (UINT32, & (PLATFORM_PMC_BASE -> PCER), 0x00000200UL);

  /*
   * Send a SW reset.
   */

	cpu_write (UINT32, & (PLATFORM_MCI_BASE -> CR), 0x00000080UL);

  /*
   * Set various modes:
   * - RDPROOF and WRPROOF to 0
   * - PDCFBYTE to 0
   * - PDCPADV to 0
   * - PDCMODE to 0
   */

  clock_divider = (master_clock / (MCI_INITIAL_SPEED * 2)) - 1;
  clock_divider += (master_clock % (2 * MCI_INITIAL_SPEED) != 0) ? 1 : 0;

  mode |= clock_divider;
  mode |= 0x1F << 8;
  mode |= 512 << 16;
	cpu_write (UINT32, & (PLATFORM_MCI_BASE -> MR), mode);

  /*
   * Set the data timeout to 1 second.
   * Configuration is as follows: k x (1 << 20) cycles.
   */

  dtoconfig |= master_clock >> 20;
	cpu_write (UINT32, & (PLATFORM_MCI_BASE -> DTR), dtoconfig);

  /*
   * Set SD slot A and 4-bit data bus.
   */

	cpu_write (UINT32, & (PLATFORM_MCI_BASE -> SDCR), 0x00000080UL);

  /*
   * Enable interrupts.
   */

	cpu_write (UINT32, & (PLATFORM_MCI_BASE -> IER), 0x00800000UL);

  /*
   * Enable the device.
   */

	cpu_write (UINT32, & (PLATFORM_MCI_BASE -> CR), 0x00000001UL);
  return DNA_OK;
}

