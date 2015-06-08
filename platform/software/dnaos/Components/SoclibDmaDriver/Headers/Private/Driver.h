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

#ifndef SOCLIB_DMA_DRIVER_PRIVATE_H
#define SOCLIB_DMA_DRIVER_PRIVATE_H

#include <DnaTools/DnaTools.h>
#include <Private/Dma.h>

extern status_t soclib_dma_init_hardware(void);
extern status_t soclib_dma_init_driver(void);
extern void soclib_dma_uninit_driver(void);
extern const char **soclib_dma_publish_devices(void);
extern device_cmd_t *soclib_dma_find_device(const char *name);

extern const char *soclib_dma_devices[];

#endif
