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

#ifndef MULTIMEDIACARD_CID_PRIVATE_H
#define MULTIMEDIACARD_CID_PRIVATE_H

#include <stdint.h>

#define MANUFACTURER_ID(x)    (x & (0xFFUL     << 24)) >> 24
#define OEM_ID(x)             (x & (0xFFFFUL   <<  8)) >>  8
#define PRODUCT_NAME_C1(x)    (x & (0xFFUL     <<  0)) >>  0
#define PRODUCT_NAME_C2(x)    (x & (0xFFUL     << 24)) >> 24 
#define PRODUCT_NAME_C3(x)    (x & (0xFFUL     << 16)) >> 16
#define PRODUCT_NAME_C4(x)    (x & (0xFFUL     <<  8)) >>  8
#define PRODUCT_NAME_C5(x)    (x & (0xFFUL     <<  0)) >>  0
#define PRODUCT_REVISION(x)   (x & (0xFFUL     << 24)) >> 24
#define SERIAL_NUMBER(x,y)    (x & (0xFFFFFFUL <<  0)) <<  8 | \
                              (y & (0xFFUL     << 24)) >> 24
#define MANUFACTURING_DATE(x) (x & (0xFFFUL    <<  8)) >>  8

typedef struct _mmc_cid
{
  char name[6];
  uint8_t manufacturer_id;
  uint16_t oem_id;
  uint8_t revision;
  uint32_t serial_number;
  uint16_t manufacturing_date;
}
mmc_cid_t;

extern void mmc_build_cid (uint32_t response[4], mmc_cid_t * cid);
extern void mmc_show_cid (mmc_cid_t cid);

#endif

