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
#include <Processor/Processor.h>

/**
 * soclib_dma_isr: interrupt service routine
 * @data: interrupt number
 */
int32_t soclib_dma_isr(void *data)
{
	int32_t i;
	soclib_dma_t *dma;

	for (i = 0; i < SOCLIB_DMA_NDEV; ++i)
		if (DMA[i].irq == (int32_t)data)
			break;

	if (i == SOCLIB_DMA_NDEV)
		return DNA_UNHANDLED_INTERRUPT;
	else
		dma = &DMA[i];

	/*
	 * Acknowledge the DMA IRQ by resetting the controller (the only way
	 * to clear the IRQ line of SoCLib DMA controller).
	 */
	cpu_write(UINT32, dma->regs + RESET, 1);

	/*
	 * Acknowledging the DMA IRQ also clears the interrupt enable signal (an
	 * abnormal behaviour of the SoCLib DMA controller). Hence, we need to
	 * set that signal again.
	 */
	cpu_write(UINT32, dma->regs + IRQ_DISABLED, 0);

	/*
	 * Remove one entry from the ring buffer on completion of a DMA
	 * transaction.
	 */
	dma->start = (dma->start + 1) % BUFFER_SIZE;

	/* Reprogram the channel if the ring buffer is not empty */
	if (dma->start != dma->end) {
		cpu_write(UINT32, dma->regs + SRC, dma->buf[dma->start].src);
		cpu_write(UINT32, dma->regs + DST, dma->buf[dma->start].dst);
		cpu_write(UINT32, dma->regs + LEN, dma->buf[dma->start].len);
	}

	semaphore_release(dma->sem_id, 1, DNA_NO_RESCHEDULE);

	return DNA_INVOKE_SCHEDULER;
}
