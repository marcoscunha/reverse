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

#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>
#include <Private/Driver.h>

void soclib_dma_uninit_driver(void)
{
	for (int32_t i = 0; i < SOCLIB_DMA_NDEV; ++i) {
		interrupt_detach(0, DMA[i].irq, soclib_dma_isr);
		semaphore_destroy(DMA[i].sem_id);
	}

	kernel_free(DMA);
}
