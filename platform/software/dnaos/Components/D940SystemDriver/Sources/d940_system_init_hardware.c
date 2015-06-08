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
#include <Private/Driver.h>
#include <DnaTools/DnaTools.h>
#include <Platform/Platform.h>

status_t d940_system_init_hardware (void)
{
  uint32_t status = 0, main_freq = 0, pll_clock = 0, master_clock = 0;
	uint32_t plla_config = 0, plla_mula = 0, plla_diva = 0;
  uint32_t clock_freq = 0, clock_tick = 0, baud_rate = 0;
  uint32_t clock_divider = 0;

	/*
	 * Initializing the PMC :
	 * - Enable the main oscillator
	 * - Start it after 7 * 16 (56) slow clock cycles
	 */

	cpu_write (UINT32, & (PLATFORM_PMC_BASE -> MOR), 0x00000701);

	/*
	 * - Wait for the oscillator to be ready
	 * - Check the oscillator frequency
	 */

	do
  {
    cpu_read (UINT32, & (PLATFORM_PMC_BASE -> MCFR), status);
  }
	while (! (status & 0x00010000));

	/*
	 * Initializing PLLA to output 100 Mhz
	 * - Set bit 29 to 1 (we're programming the register)
	 * - Set multiplier to PLLA_MULA
	 * - Lock after 5 slow clock cycle
	 * - Set divider to 3
	 * - Wait for the PLL to be ready
	 */

  plla_config = 0x20000503;
  plla_config |= PLLA_MULA << 16;
	cpu_write (UINT32, & (PLATFORM_PMC_BASE -> PLLAR), plla_config);

	do
  {
    cpu_read (UINT32, & (PLATFORM_PMC_BASE -> SR), status);
  }
	while (! (status & 0x00000002));

	/*
	 * Initializing master clock and processor clock
	 * - Set the master clock to main clock prescaler to 1
	 * - Set the master clock division to 2
	 * - Set the master clock to PLLA
	 * - Wait for the master clock to stabilize
	 */

	cpu_write (UINT32, & (PLATFORM_PMC_BASE -> MCR), 0x00000102);

	do
  {
    cpu_read (UINT32, & (PLATFORM_PMC_BASE -> SR), status);
  }
	while (! (status & 0x00000008));

  /*
   * Read various clock parameters:
   * - Read the main frequency.
   * - Read the PLLA configuration.
   * - Compute the master clock.
   */

	cpu_read (UINT32, & (PLATFORM_PMC_BASE -> MCFR), main_freq);
	main_freq = ((uint32_t)(main_freq & 0x0000FFFF)) << 11;

	cpu_read (UINT32, & (PLATFORM_PMC_BASE -> PLLAR), plla_config);
	plla_mula = ((uint32_t)(plla_config & 0x07FF0000)) >> 16;
	plla_diva = plla_config & 0x0000000F;

	pll_clock = (plla_mula * main_freq) / plla_diva;
	master_clock = pll_clock >> 1;

  log (INFO_LEVEL, "master clock = %d", master_clock);

  /*
   * Enable PIOx clocks.
   */

	cpu_write (UINT32, & (PLATFORM_PMC_BASE -> PCER), 0x0000001C);

	/*
	 * Configure the PLATFORM_DBGU_BASE :
	 * - Reset transmitter and receiver
	 * - Enable Receiver
	 * - Enable transmitter with a baud rate ratio of 36 (@ 160Mhz)
	 * - Enable RX
	 */

  do cpu_read (UINT32, & (PLATFORM_DBGU_BASE -> SR), status);
  while ((status & 0x202) != 0x202);

	cpu_write (UINT32, & (PLATFORM_DBGU_BASE -> CR), 0x0000010C);
	cpu_write (UINT32, & (PLATFORM_DBGU_BASE -> MR), 0x00000800);
	cpu_write (UINT32, & (PLATFORM_DBGU_BASE -> IER), 0x00000001);

  baud_rate = master_clock / (115200 * 16);
	cpu_write (UINT32, & (PLATFORM_DBGU_BASE -> BDCR), baud_rate);
	cpu_write (UINT32, & (PLATFORM_DBGU_BASE -> CR), 0x00000050);

	/*
	 * Compute the rate of the PIT and program it.
	 */

	clock_freq = master_clock >> 4;
  log (INFO_LEVEL, "timer clock freq = %d", clock_freq);

  /*
   * Set the default tick timer @ 1ms (1000 Hz).
   */

  clock_divider = (clock_freq / 1000);
  log (INFO_LEVEL, "timer clock divider = %d", clock_divider);
	clock_tick = 0x03000000 | clock_divider;

	cpu_write (UINT32, & (PLATFORM_SYSC_BASE -> PIT_MODE), clock_tick);
  return DNA_OK;
}

