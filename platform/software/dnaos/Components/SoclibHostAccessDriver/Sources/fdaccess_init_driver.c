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

status_t fdaccess_init_driver (void)
{
  fdaccess_config_t config;
  int32_t total_names = 1, name_index = 0;
  char * base_path = "disk/simulator/", * device_base, ascii[64];

  watch (status_t)
  {
    /*
     * Compute the total number of entries.
     */

    for (int32_t i = 0; i < SOCLIB_FDACCESS_NDEV; i += 1)
    {
      total_names += SOCLIB_FDACCESS_DEVICES[i] . entries;
    }

    fdaccess_devices = kernel_malloc (total_names * sizeof (char *), true);
    ensure (fdaccess_devices != NULL, DNA_OUT_OF_MEM);

    /*
     * Create the associated path names.
     */
  
    for (int32_t i = 0; i < SOCLIB_FDACCESS_NDEV; i += 1)
    {
      device_base = kernel_malloc (DNA_PATH_LENGTH, false);
      check (no_memory, device_base != NULL, DNA_OUT_OF_MEM);

      config = SOCLIB_FDACCESS_DEVICES[i];
      dna_strcpy (device_base, (const char *)base_path);
      dna_itoa (i, ascii);
      dna_strcat (device_base, ascii);
      dna_strcat (device_base, "/");

      for (int32_t j = 0; j < config . entries; j += 1)
      {
        fdaccess_devices[name_index] = kernel_malloc (DNA_PATH_LENGTH, false);
        check (no_memory, fdaccess_devices[name_index] != NULL, DNA_OUT_OF_MEM);

        dna_strcpy (fdaccess_devices[name_index], (const char *)device_base);
        dna_itoa (j, ascii);
        dna_strcat (fdaccess_devices[name_index], ascii);

        name_index += 1;
      }

      kernel_free (device_base);
    }

    return DNA_OK;
  }

  rescue (no_memory)
  {
    for (int32_t i = 0; i <= name_index; i += 1)
    {
      if (fdaccess_devices[i] != NULL)
      {
        kernel_free (fdaccess_devices[i]);
      }
    }

    if (device_base != NULL)
    {
      kernel_free (device_base);
    }

    kernel_free (fdaccess_devices);
    leave;
  }
}

