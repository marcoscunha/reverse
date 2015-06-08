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

#ifndef MULTIMEDIACARD_OCR_PRIVATE_H
#define MULTIMEDIACARD_OCR_PRIVATE_H

#include <stdint.h>

#define MMC_VDD_16_17          (1 << 4)
#define MMC_VDD_17_18          (1 << 5)
#define MMC_VDD_18_19          (1 << 6)
#define MMC_VDD_19_20          (1 << 7)
#define MMC_VDD_20_21          (1 << 8)
#define MMC_VDD_21_22          (1 << 9)
#define MMC_VDD_22_23          (1 << 10)
#define MMC_VDD_23_24          (1 << 11)
#define MMC_VDD_24_25          (1 << 12)
#define MMC_VDD_25_26          (1 << 13)
#define MMC_VDD_26_27          (1 << 14)
#define MMC_VDD_27_28          (1 << 15)
#define MMC_VDD_28_29          (1 << 16)
#define MMC_VDD_29_30          (1 << 17)
#define MMC_VDD_30_31          (1 << 18)
#define MMC_VDD_31_32          (1 << 19)
#define MMC_VDD_32_33          (1 << 20)
#define MMC_VDD_33_34          (1 << 21)
#define MMC_VDD_34_35          (1 << 22)
#define MMC_VDD_35_36          (1 << 23)
#define MMC_CARD_POWER_UP_BUSY (1 << 31)
#define MMC_MMC_OCR_BIT2930    (3 << 29)
#define MMC_SD_HCS             (1 << 30)

#define MMC_VOLTAGE_RANGE                         \
  MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 | \
  MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33

typedef union _mmc_ocr
{
  uint32_t raw;

  struct _mmc_ocr_bits
  {
    uint32_t reserved_0      :15;
    uint32_t voltage_27_28   :1;
    uint32_t voltage_28_29   :1;
    uint32_t voltage_29_30   :1;
    uint32_t voltage_30_31   :1;
    uint32_t voltage_31_32   :1;
    uint32_t voltage_32_33   :1;
    uint32_t voltage_33_34   :1;
    uint32_t voltage_34_35   :1;
    uint32_t voltage_35_36   :1;
    uint32_t reserved_1      :6;
    uint32_t capacity_status :1;
    uint32_t not_busy        :1;
  }
  bits;
}
mmc_ocr_t;

extern void mmc_show_ocr (mmc_ocr_t ocr);

#endif

