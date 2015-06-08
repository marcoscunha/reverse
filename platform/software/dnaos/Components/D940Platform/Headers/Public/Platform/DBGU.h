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

#ifndef PLATFORM_DBGU_H
#define PLATFORM_DBGU_H

#include <stdint.h>
#include <Platform/PDC.h>

/*
 * Definition of the device itself.
 */

typedef volatile struct _d940_dbgu
{
	uint32_t CR;
	uint32_t MR;
	uint32_t IER;
	uint32_t IDR;
	uint32_t IMR;
	uint32_t SR;
	uint32_t RHR;
	uint32_t THR;
	uint32_t BDCR;
	uint32_t RESERVED_0[7];
	uint32_t CIDR;
	uint32_t CIDER;
	uint32_t FNR;
	uint32_t RESERVED_1[45];
	d940_pdc_t pdc;
}
* d940_dbgu_t;

enum
{
	DBGU_CR_RESET_RECEIVER 				= 0x004,
	DBGU_CR_RESET_TRANSMITTER			= 0x008,
	DBGU_CR_RECEIVER_ENABLE 			= 0x010,
	DBGU_CR_RECEIVER_DISABLE 			= 0x020,
	DBGU_CR_TRANSMITTER_ENABLE 		= 0x040,
	DBGU_CR_TRANSMITTER_DISABLE 	= 0x080,
	DBGU_CR_RESET_STATUS_BIT			= 0x100
};

enum
{
	DBGU_MR_PARITY_EVEN 						= 0xFFFFF1FF,
	DBGU_MR_PARITY_ODD							= 0x00000200,
	DBGU_MR_PARITY_SPACE 						= 0x00000400,
	DBGU_MR_PARITY_MARK 						= 0x00000600,
	DBGU_MR_PARITY_NONE 						= 0x00000800,
	DBGU_MR_MODE_NORMAL 						= 0xFFFF3FFF,
	DBGU_MR_MODE_AUTOECHO 					= 0x00004000,
	DBGU_MR_MODE_LOCAL_LOOPBACK 		= 0x00008000,
	DBGU_MR_MODE_REMOTE_LOOPBACK 		= 0x00008000
};

enum
{
	DBGU_IEDR_RXRDY			= 0x00000001,
	DBGU_IEDR_TXRDY			= 0x00000002,
	DBGU_IEDR_ENDRX			= 0x00000008,
	DBGU_IEDR_ENDTX			= 0x00000010,
	DBGU_IEDR_OVRE			= 0x00000020,
	DBGU_IEDR_FRAME 		= 0x00000040,
	DBGU_IEDR_PARE			= 0x00000080,
	DBGU_IEDR_TXEMPTY		= 0x00000200,
	DBGU_IEDR_TXBUFE		= 0x00000800,
	DBGU_IEDR_RXBUFF		= 0x00001000,
	DBGU_IEDR_COMMTX		= 0x40000000,
	DBGU_IEDR_COMMRX		= 0x80000000
};

extern d940_dbgu_t PLATFORM_DBGU_BASE;

#endif

