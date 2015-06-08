/*
 * Copyright (C) 2009-2010 TIMA Laboratory
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
#include <Processor/Processor.h>

/**
 * soclib_dma_init_hardware: boot time initialization
 *
 * Configure DMA controller to enable interrupt.
 */
status_t soclib_dma_init_hardware(void)
{
	for (int32_t i = 0; i < SOCLIB_DMA_NDEV; ++i)
		cpu_write(UINT32, SOCLIB_DMA_DEVICES[i].regs + IRQ_DISABLED, 0);

	return DNA_OK;
}
