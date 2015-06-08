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

#ifndef MULTIMEDIACARD_RCA_PRIVATE_H
#define MULTIMEDIACARD_RCA_PRIVATE_H

#include <stdint.h>

typedef union _mmc_rca
{
  uint32_t raw;

  struct _mmc_rca_bits
  {
    uint32_t test_mode            :2;
    uint32_t app_specific_command :2;
    uint32_t reserved_0           :1;
    uint32_t application_command  :1;
    uint32_t reserved_1           :1;
    uint32_t switch_error         :1;
    uint32_t ready_for_data       :1;
    uint32_t current_state        :4;
    uint32_t error                :1;
    uint32_t illegal_command      :1;
    uint32_t com_crc_error        :1;
    uint32_t rca                  :16;
  }
  bits;
}
mmc_rca_t;

extern void mmc_show_rca (mmc_rca_t rca);

#endif

