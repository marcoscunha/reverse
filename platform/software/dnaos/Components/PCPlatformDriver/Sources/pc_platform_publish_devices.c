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

const char ** pc_platform_publish_devices (void)
{
  char * base_path = "serial/pc/", * device_base, alpha_index[8];

  watch (const char **)
  {
    if (pc_platform_devices != NULL)
    {
      for (int32_t i = 0; pc_platform_devices[i] != NULL; i += 1)
      {
        kernel_free (pc_platform_devices[i]);
      }

      kernel_free (pc_platform_devices);
      pc_platform_devices = NULL;
    }

    /*
     * Generate the name database.
     */

    pc_platform_devices = kernel_malloc
      ((PC_TTY_NDEV + 2) * sizeof (char *), true);
    ensure (pc_platform_devices != NULL, NULL);

    for (uint32_t i = 0; i < PC_TTY_NDEV; i += 1)
    {
      device_base = kernel_malloc (DNA_PATH_LENGTH, false);
      check (no_memory, device_base != NULL, NULL);

      dna_itoa (i, alpha_index);
      dna_strcpy (device_base, base_path);
      dna_strcat (device_base, alpha_index);
      pc_platform_devices[i] = device_base;
    }

    /*
     * Add the kernel serial devices.
     * TODO find a better way to do that => pass this info as a parameter.
     */

    device_base = kernel_malloc (DNA_PATH_LENGTH, false);
    check (no_memory, device_base != NULL, NULL);

    dna_strcpy (device_base, "serial/kernel/console");
    pc_platform_devices[PC_TTY_NDEV] = device_base;

    return (const char **) pc_platform_devices;
  }

  rescue (no_memory)
  {
    for (int32_t i = 0; pc_platform_devices[i] != NULL; i += 1)
    {
      kernel_free (pc_platform_devices[i]);
    }

    kernel_free (pc_platform_devices);
    pc_platform_devices = NULL;

    leave;
  }
}

