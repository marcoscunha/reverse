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

#ifndef D940_MMC_STATUS_PRIVATE_H
#define D940_MMC_STATUS_PRIVATE_H

#include <stdint.h>

#define STATUS_ERROR_MASK 0xC07B0000

typedef union _d940_mmc_status
{
  uint32_t raw;

  struct _mmc_status_bits
  {
    uint32_t command_ready             : 1;
    uint32_t receiver_ready            : 1;
    uint32_t transmitter_ready         : 1;
    uint32_t data_block_ended          : 1;
    uint32_t data_transfer_in_progress : 1;
    uint32_t mci_not_busy              : 1;
    uint32_t end_rx_buffer             : 1;
    uint32_t end_tx_buffer             : 1;
    uint32_t sdio_irqa                 : 1;
    uint32_t pad_0                     : 5;
    uint32_t rx_buffer_full            : 1;
    uint32_t tx_buffer_empty           : 1;
    uint32_t response_index_error      : 1;
    uint32_t response_direction_error  : 1;
    uint32_t response_crc_error        : 1;
    uint32_t response_end_bit_error    : 1;
    uint32_t response_time_out_error   : 1;
    uint32_t data_crc_error            : 1;
    uint32_t data_time_out_error       : 1;
    uint32_t pad_1                     : 7;
    uint32_t overrun                   : 1;
    uint32_t underrun                  : 1;
  }
  bits;
}
d940_mmc_status_t;

extern void d940_mmc_show_status (d940_mmc_status_t status);

#endif
