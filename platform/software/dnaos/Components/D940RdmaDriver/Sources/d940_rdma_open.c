/************************************************************************
 * rdma_driver_driver.c : DNA rdma_driver driver                                    *
 * Copyright (C) 2008 TIMA Laboratory                                    *
 * Author: Alexandre CHAGOYA-GARZON
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
#include <Private/Dnp.h>

status_t d940_rdma_open (const char * name, int32_t mode, void ** data) {
        int channel_id;

        // pff... 
        strtok((char *)name, ".");
        channel_id = atoi((char *)strtok(NULL, "."));
    
	*data = (void *) &RDMA_CHANNELS[channel_id];

        dnp_rdma_open(RDMA_CHANNELS[channel_id] . id);

	return DNA_OK;
}



