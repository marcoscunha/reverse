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

#include <Private/Core.h>
#include <DnaTools/DnaTools.h>
#include <MemoryManager/MemoryManager.h>
#include <Processor/Processor.h>

/****f* Interrupt/interrupt_attach
 * SUMMARY
 * Attach a interrupt to an ID.
 *
 * SYNOPSIS
 */

status_t interrupt_attach (int32_t cpuid, interrupt_id_t id,
    int32_t mode, interrupt_handler_t handler, bool bypass_demux)

/*
 * ARGUMENTS
 * * cpuid : the ID of the target processor
 * * id : the ID of the interrupt to attach
 * * mode : the mode of the attach
 * * handler : handler of the interrupt
 * * bypass_demux : handler has to be installed directly
 *
 * RESULT
 * * DNA_BAD_ARGUMENT: on of the arguments is not correct
 * * DNA_OUT_OF_MEM: cannot allocate the necessary memory to create the ISR
 * * DNA_ERROR: there are more than one ISR and bypass_demux is set
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  isr_t isr = NULL;
  queue_t * queue = NULL;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (id < cpu_trap_count (), DNA_BAD_ARGUMENT);
    ensure (cpuid < cpu_mp_count (), DNA_BAD_ARGUMENT);

    /*
     * Create the new ISR
     */

    isr = kernel_malloc (sizeof (struct _isr), true);
    ensure (isr != NULL, DNA_OUT_OF_MEM);

    isr -> handler = handler;

    /*
     * Add the new ISR in the appropriate queue
     */

    it_status = cpu_trap_mask_and_backup();
    queue = & cpu_pool . cpu[cpuid] . isr[id];

    lock_acquire (& queue -> lock);
    queue_add (queue, isr);

    check (not_alone, ! bypass_demux ||
        (bypass_demux && queue -> status == 1), DNA_ERROR);

    if (queue -> status == 1)
    {
      if (bypass_demux)
      {
        cpu_trap_attach_isr (cpuid, id, mode, handler);
      }
      else
      {
        cpu_trap_attach_isr (cpuid, id, mode, interrupt_demultiplexer);
      }

      if (cpuid == cpu_mp_id ())
      {
        cpu_trap_enable (id);
      }
      else
      {
        lock_acquire (& cpu_pool . cpu[cpuid] . ipi_lock);
        cpu_mp_send_ipi (cpuid, DNA_IPI_TRAP_ENABLE, (void *) id);
      }
    }

    lock_release (& queue -> lock);
    cpu_trap_restore(it_status);

    return DNA_OK;
  }

  rescue (not_alone)
  {
    queue_extract (queue, isr);
    lock_release (& queue -> lock);
    cpu_trap_restore(it_status);

    kernel_free (isr);
    leave;
  }
}

/*
 * NOTES
 * The demux bypass flag should only be used with care. The handler should be
 * the only one on the line, and it no result can be expected by returning
 * DNA_INVOKE_SCHEDULER, since this is handle by the demuxer. Last but not
 * least, the CPU status is not updated if the demuxer is bypassed, meaning that
 * the CPU is still eligble for dispatch.
 ****/

