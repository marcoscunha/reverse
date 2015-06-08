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

#ifndef D940_MMC_DEVICE_PRIVATE_H
#define D940_MMC_DEVICE_PRIVATE_H

#include <stdint.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>
#include <MultiMediaCard/MultiMediaCard.h>
#include <Platform/MCI.h>

typedef struct _mmc_driver
{
  spinlock_t lock;
  int32_t connection_alarm;
  mmc_extension_t * mmc;
  mmc_card_t card;
}
mmc_driver_t;

extern mmc_driver_t d940_mmc_driver;

extern status_t d940_mmc_open (char * name, int32_t mode, void ** data);
extern status_t d940_mmc_close (void * data);
extern status_t d940_mmc_free (void * data);

extern status_t d940_mmc_read (void * handler, void * destination,
    int64_t offset, int32_t * p_count);
extern status_t d940_mmc_write (void * handler, void * source,
    int64_t offset, int32_t * p_count);
extern status_t d940_mmc_control (void * handler, int32_t operation,
    void * data, int32_t * p_res);

extern int32_t d940_mmc_isr (void * data);

#endif

