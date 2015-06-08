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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
 * GNU General Public License for more details.                          
 *                                                                       
 * You should have received a copy of the GNU General Public License     
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include <Private/Driver.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

status_t rdv_init_driver (void)
{
  char * base = "channel/rendezvous/", * buffer, ascii[64];
  int32_t i = 0;
  status_t status;

  watch (status_t)
  {
    rdv_devices = kernel_malloc (sizeof (const char *)
        * (CHANNEL_RDV_NDEV + 1), true);
    ensure (rdv_devices != NULL, DNA_OUT_OF_MEM);

    rdv = kernel_malloc (sizeof (channel_rdv_t) * (CHANNEL_RDV_NDEV), true);
    ensure (rdv_devices != NULL, DNA_OUT_OF_MEM);

    for (i = 0; i < CHANNEL_RDV_NDEV; i++)
    {
      /*
       * Create the channel name
       */

      buffer = kernel_malloc (DNA_FILENAME_LENGTH, false);
      check (invalid_buffer, buffer != NULL, DNA_OUT_OF_MEM);
      
      dna_itoa (i, ascii);
      dna_strcpy (buffer, base);
      dna_strcat (buffer, ascii);
      rdv_devices[i] = buffer;

      /*
       * Create the channel semaphore
       */

      status = semaphore_create (buffer, 0, & rdv[i] . sem);
      ensure (status == DNA_OK, status);
    }

    rdv_devices[i] = NULL;
    return DNA_OK;
  }

  rescue (invalid_buffer)
  {
    for (i = 0; i < CHANNEL_RDV_NDEV; i++)
    {
      if (rdv_devices[i] != NULL) kernel_free ((void *)rdv_devices[i]);
    }

    kernel_free (rdv_devices);
    leave;
  }
}

