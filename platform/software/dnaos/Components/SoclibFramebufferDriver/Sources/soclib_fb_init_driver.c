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
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>
#include <Processor/Processor.h>
#include <DnaTools/DnaTools.h>

status_t soclib_fb_init_driver (void)
{
  char alpha_index[8];

  /*
   * Create the device name array
   */

  soclib_fb_devices = kernel_malloc (sizeof (char *) *
      SOCLIB_FB_NDEV + 1, true);

  /*
   * Create the FBs
   */

  FB = kernel_malloc (sizeof (soclib_framebuffer_t) * SOCLIB_FB_NDEV, true);
  if (FB == NULL) return DNA_OUT_OF_MEM;

  for (int32_t i = 0; i < SOCLIB_FB_NDEV; i += 1)
  {
    dna_itoa (i, alpha_index);
    dna_strcpy (FB[i] . name, "video/simulator/");
    dna_strcat (FB[i] . name, alpha_index);
    soclib_fb_devices[i] = FB[i] . name;
    FB[i] . config = SOCLIB_FB_DEVICES[i];
  }

  return DNA_OK;
}

