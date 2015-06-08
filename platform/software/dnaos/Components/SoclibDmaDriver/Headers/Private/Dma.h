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

#ifndef SOCLIB_DMA_H
#define SOCLIB_DMA_H

#include <DnaTools/DnaTools.h>

#define SRC		0x0
#define DST		0x4
#define LEN		0x8
#define RESET		0xC
#define IRQ_DISABLED	0x10

#define BUFFER_SIZE	32

typedef struct soclib_dma_config {
	uint32_t irq;
	void *regs;
} soclib_dma_config_t;

typedef struct tx_descriptor {
	uint32_t src;
	uint32_t dst;
	uint32_t len;
} tx_descriptor_t;

typedef struct soclib_dma {
	spinlock_t lock __attribute__((aligned(32)));
    int32_t spare[7];
	/* A ring buffer with BUFFER_SIZE slots */
	tx_descriptor_t buf[BUFFER_SIZE];
	/* Start index of the buffer */
	uint32_t start;
	/* End index of the buffer */
	uint32_t end;

	uint32_t irq;
	void *regs;
	int32_t sem_id;
} soclib_dma_t;

extern uint32_t SOCLIB_DMA_NDEV;
extern soclib_dma_config_t SOCLIB_DMA_DEVICES[];

extern soclib_dma_t *DMA;
extern device_cmd_t soclib_dma_commands;

extern int32_t soclib_dma_isr(void *data);

extern status_t soclib_dma_open(char *name, int32_t mode, void **data);
extern status_t soclib_dma_close(void *data);
extern status_t soclib_dma_free(void *data);
extern status_t soclib_dma_control(void *handler, int32_t function,
				   void *arguments, int32_t *p_ret);

#endif
