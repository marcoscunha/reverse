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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the 
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program.If not, see <http://www.gnu.org/licenses/>. 
 */

#include <stdint.h>

#ifndef PLATFORM_PDC_H
#define PLATFORM_PDC_H

typedef volatile struct _d940_pdc
{
	uint32_t RPR;    /* Receive Pointer Register */
	uint32_t RCR;    /* Receive Counter Register */
	uint32_t TPR;    /* Transmit Pointer Register */
	uint32_t TCR;    /* Transmit Counter Register */
	uint32_t RNPR;		/* Receive Next Pointer Register */
	uint32_t RNCR;		/* Receive Next Counter Register */
	uint32_t TNPR;		/* Transmit Next Pointer Register */
	uint32_t TNCR;		/* Transmit Next Counter Register */
	uint32_t PTCR;		/* Transfer Control Register */
	uint32_t PTSR;		/* Transfer Status Register */
}
d940_pdc_t;

enum
{
	PDC_PTCR_RXTEN 	= 0x00000001,
	PDC_PTCR_RXTDIS = 0x00000002,
	PDC_PTCR_TXTEN 	= 0x00000100,
	PDC_PTCR_TXTDIS = 0x00000200
};

enum
{
	PDC_PTSR_RXTEN = 0x00000001,
	PDC_PTSR_TXTEN = 0x00000100
};

#endif

