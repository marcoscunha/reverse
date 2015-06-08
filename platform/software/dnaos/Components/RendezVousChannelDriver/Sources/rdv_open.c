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

status_t rdv_open (char * name, int32_t mode, void ** data)
{
  int32_t id;

  watch (status_t)
  {
    ensure (name != NULL, DNA_ERROR);

    /*
     * Get the channel ID from the beginning
     * of the path.
     */

    id = dna_atoi (& name[19]);
    ensure (id >= 0 && id < CHANNEL_RDV_NDEV, DNA_ERROR);

    /*
     * Return the device
     */

    *data = (void *) & rdv[id];
    return DNA_OK;
  }
}

