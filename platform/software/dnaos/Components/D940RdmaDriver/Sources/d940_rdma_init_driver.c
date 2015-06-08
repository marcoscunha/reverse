/************************************************************************
 * Copyright (C) 2010 TIMA Laboratory                                    *
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
#include <MemoryManager/MemoryManager.h>
#include <Private/RdmaChannel.h>




const char **d940_rdma_devices;

driver_t d940_rdma_module = {
	"rdma",
	d940_rdma_init_hardware,
	d940_rdma_init_driver,
	d940_rdma_uninit_driver,
	d940_rdma_publish_devices,
	d940_rdma_find_device
};

status_t rdma_init_driver (void) {
	char * base = "rdma.", * buffer, ascii[64];
	int32_t i = 0;

	d940_rdma_devices = kernel_malloc (sizeof (const char *) * (RDMA_CHANNEL_NDEV + 1), false);
	kassert (d940_rdma_devices != NULL, return DNA_OUT_OF_MEM);

	for (i = 0; i < RDMA_CHANNEL_NDEV; i++) {
		buffer = kernel_malloc (DNA_FILENAME_LENGTH, false);
		kassert (buffer != NULL, return DNA_OUT_OF_MEM);
		
		dna_itoa (i, ascii);
		dna_strcpy (buffer, base);
		dna_strcat (buffer, ascii);
		d940_rdma_devices[i] = buffer;
	}

	d940_rdma_devices[i] = NULL;
	return DNA_OK;
}





