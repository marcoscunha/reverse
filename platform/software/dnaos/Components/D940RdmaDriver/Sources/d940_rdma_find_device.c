/************************************************************************
 * Copyright (C) 2008 TIMA Laboratory                                    *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include <DnaTools/DnaTools.h>
#include <Private/RdmaChannel.h>

device_cmd_t d940_rdma_commands = {
	d940_rdma_open,
	d940_rdma_close,
	d940_rdma_free,
	d940_rdma_read,
	d940_rdma_write,
	d940_rdma_control
};

device_cmd_t * d940_rdma_find_device (const char * name) {

	return & d940_rdma_commands;
}

