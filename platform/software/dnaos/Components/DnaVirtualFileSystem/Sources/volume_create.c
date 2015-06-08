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

#include <Private/VirtualFileSystem.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****f* VolumePrivate/volume_create
 * SUMMARY
 * Allocate a volume.
 *
 * SYNOPSIS
 */

status_t volume_create (volume_t host_volume, int64_t host_vnid,
    filesystem_cmd_t * cmd, volume_t * p_volume)

/*
 * ARGUMENTS
 * * vid : the ID of the volume
 * * host_volume : a pointer to a volume_t;
 * * host_vnid : a pointer to a volume_t;
 * * cmd : a pointer to a volume_t;
 * * volume : a pointer to a volume_t;
 *
 * FUNCTION
 * * Allocate the necessary memory for a volume_t element.
 *
 * RESULT
 * DNA_OK if th operation succedded, DNA_ERROR otherwise.
 *
 * SOURCE
 */

{
  volume_t volume = NULL;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    volume = kernel_malloc (sizeof (struct _volume), true);
    ensure (volume != NULL, DNA_OUT_OF_MEM);

    volume -> id = atomic_add (& volume_manager . volume_index, 1);
    volume -> root_vnid = -1;
    volume -> host_volume = host_volume;
    volume -> host_vnid = host_vnid;
    volume -> cmd = cmd;
    DCACHE_FLUSH(volume,sizeof(struct _volume));


    dna_printf("@@@@@@@@@@@@@@@@@@@@@@@@@@\n");


    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& volume_manager . volume_list . lock);

    queue_add (& volume_manager . volume_list, volume);

    lock_release (& volume_manager . volume_list . lock);
    cpu_trap_restore(it_status);

    *p_volume = volume;
    return DNA_OK;
  }
}

/*
 ****/

