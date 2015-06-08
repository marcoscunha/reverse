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
 * soclib_dma_module: driver initialization
 *
 * Takes place at boot time once and for all
 */
driver_t soclib_dma_module = {
	"soclib_dma",
	soclib_dma_init_hardware,
	soclib_dma_init_driver,
	soclib_dma_uninit_driver,
	soclib_dma_publish_devices,
	soclib_dma_find_device
};

device_cmd_t soclib_dma_commands = {
	soclib_dma_open,
	soclib_dma_close,
	soclib_dma_free,
	NULL,
	NULL,
	soclib_dma_control
};

/* Array of device names */
const char *soclib_dma_devices[] = {
	"channel/soclib/0/0",
	NULL
};
