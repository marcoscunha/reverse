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
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>

soclib_dma_t *DMA;

/**
 * soclib_dma_init_driver: instantiate the DMA devices
 *
 * Creation of device names goes here or in the xx_variable.c file. Specifying
 * device names enables devices to be accessible in the /devices directory.
 */
status_t soclib_dma_init_driver(void)
{
	char sem_name[64];
	char alpha_index[8];
	status_t ret;

	/* Create the DMAs */
	DMA = kernel_malloc(sizeof(soclib_dma_t) * SOCLIB_DMA_NDEV, true);
	if (!DMA)
		return DNA_OUT_OF_MEM;

	for (int32_t i = 0; i < SOCLIB_DMA_NDEV; ++i) {
		DMA[i].irq = SOCLIB_DMA_DEVICES[i].irq;
		DMA[i].regs = SOCLIB_DMA_DEVICES[i].regs;

		DMA[i].start = DMA[i].end = 0;

		dna_itoa(i, alpha_index);
		dna_strcpy(sem_name, "soclib_dma_");
		dna_strcat(sem_name, alpha_index);
		dna_strcat(sem_name, "_sem");

		ret = semaphore_create(sem_name, 1, &DMA[i].sem_id);
		if (ret) {
 dna_log(INFO_LEVEL, "Semaphore creation failed");
			goto sem_error;
		}

		ret = interrupt_attach(0, DMA[i].irq, 0, soclib_dma_isr, false);
		if (ret) {
 dna_log(INFO_LEVEL, "Couldn't claim IRQ %u", DMA[i].irq);
			goto irq_error;
		}
	}

	return DNA_OK;

irq_error:
sem_error:
	for (int32_t i = 0; DMA[i].sem_id != 0; ++i)
		semaphore_destroy(DMA[i].sem_id);

	kernel_free(DMA);
	return ret;
}
