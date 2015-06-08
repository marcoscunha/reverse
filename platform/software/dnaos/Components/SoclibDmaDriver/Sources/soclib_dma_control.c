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
#include <SoclibDmaDriver/Driver.h>
#include <Core/Core.h>
#include <Processor/Processor.h>

/**
 * soclib_dma_control: I/O control functions for programming the DMA controller
 * @handler: pointer to data of soclib_dma_t type
 * @function: device-dependent request function
 * @arguments: variable argument list. Data comes in this order: source address,
 *	destination address, length
 * @p_ret: pointer to a return value. Valid function request returns 0.
 */
status_t soclib_dma_control(void *handler, int32_t function, va_list arguments,
			    int32_t *p_ret)
{
	soclib_dma_t *dma = (soclib_dma_t *)handler;
	interrupt_status_t it_status = cpu_trap_mask_and_backup();
	uint32_t value;
	uint32_t *ptr;

	switch (function) {
	case XFER:
		/* If the buffer is empty, program the channel */
		if (dma->start == dma->end) {
			semaphore_acquire(dma->sem_id, 1, 0, -1);

			dma->buf[dma->end].src = va_arg(arguments, uint32_t);
			dma->buf[dma->end].dst = va_arg(arguments, uint32_t);
			dma->buf[dma->end].len = va_arg(arguments, uint32_t);

			dma->end = (dma->end + 1) % BUFFER_SIZE;

			lock_acquire(&dma->lock);

			cpu_write(UINT32, dma->regs + SRC,
				  dma->buf[dma->start].src);
			cpu_write(UINT32, dma->regs + DST,
				  dma->buf[dma->start].dst);
			cpu_write(UINT32, dma->regs + LEN,
				  dma->buf[dma->start].len);

			lock_release(&dma->lock);
		} else if ((dma->end + 1) % BUFFER_SIZE != dma->start) {
			dma->buf[dma->end].src = va_arg(arguments, uint32_t);
			dma->buf[dma->end].dst = va_arg(arguments, uint32_t);
			dma->buf[dma->end].len = va_arg(arguments, uint32_t);

			dma->end = (dma->end + 1) % BUFFER_SIZE;
		} else {
 dna_log(INFO_LEVEL, "DMA ring buffer overflow!");
		}

		*p_ret = 0;
		break;
	case GET_SRC_ADDR:
		ptr = va_arg(arguments, uint32_t *);

		lock_acquire(&dma->lock);
		cpu_read(UINT32, dma->regs + SRC, *ptr);
		lock_release(&dma->lock);

		*p_ret = 0;
		break;
	case GET_DST_ADDR:
		ptr = va_arg(arguments, uint32_t *);

		lock_acquire(&dma->lock);
		cpu_read(UINT32, dma->regs + DST, *ptr);
		lock_release(&dma->lock);

		*p_ret = 0;
		break;
	case GET_STATUS:
		ptr = va_arg(arguments, uint32_t *);

		lock_acquire(&dma->lock);
		cpu_read(UINT32, dma->regs + LEN, *ptr);
		lock_release(&dma->lock);

		*p_ret = 0;
		break;
	case SET_RESET:
		value = va_arg(arguments, uint32_t);

		lock_acquire(&dma->lock);
		cpu_write(UINT32, dma->regs + RESET, value);
		lock_release(&dma->lock);

		*p_ret = 0;
		break;
	case SET_IRQ_DISABLED:
		value = va_arg(arguments, uint32_t);

		lock_acquire(&dma->lock);
		cpu_write(UINT32, dma->regs + IRQ_DISABLED, value);
		lock_release(&dma->lock);

		*p_ret = 0;
		break;
	default:
 dna_log(INFO_LEVEL, "Unsupported control code 0x%x.", function);
		*p_ret = -1;
		break;
	}

	cpu_trap_restore(it_status);

	return DNA_OK;
}
