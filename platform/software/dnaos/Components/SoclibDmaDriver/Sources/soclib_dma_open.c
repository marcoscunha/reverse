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

/**
 * soclib_dma_open: according to device name, pass DMA data to a handler
 */
status_t soclib_dma_open(char *name, int32_t mode, void **data)
{
	int32_t i;

	if (!data)
		return DNA_ERROR;

	for (i = 0; i < SOCLIB_DMA_NDEV; ++i)
		if (dna_strcmp(name, soclib_dma_devices[i]) == 0)
			break;

	if (i == SOCLIB_DMA_NDEV)
		return DNA_ERROR;

	*data = (void *)&DMA[i];
	return DNA_OK;
}
