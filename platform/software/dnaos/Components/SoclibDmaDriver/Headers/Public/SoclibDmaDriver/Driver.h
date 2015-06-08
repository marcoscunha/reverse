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

#ifndef SOCLIB_DMA_DRIVER_H
#define SOCLIB_DMA_DRIVER_H

#include <DnaTools/DnaTools.h>

extern driver_t soclib_dma_module;

enum dma_requests {
	XFER = DNA_CONTROL_CODES_END,
	GET_SRC_ADDR,
	GET_DST_ADDR,
	GET_STATUS,
	SET_RESET,
	SET_IRQ_DISABLED
};

#endif
