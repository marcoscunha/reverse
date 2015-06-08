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

/****f* Interrupt/interrupt_detach
 * SUMMARY
 * Detach an interrupt.
 *
 * SYNOPSIS
 */

status_t interrupt_detach (int32_t cpuid, interrupt_id_t id,
    interrupt_handler_t handler)

/*
 * ARGUMENTS
 * * cpuid : id of the target CPU
 * * id : an interrupt ID
 * * handler : handler of the interrupt
 *
 * RESULT
 * * DNA_BAD_ARGUMENT: one of the arguments is incorrect
 * * DNA_OK: the operation is successful
 *
 * SOURCE
 */

{
  isr_t isr = NULL;
  queue_t * queue = NULL;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (id < CPU_TRAP_COUNT, DNA_BAD_ARGUMENT);
    ensure (cpuid < cpu_mp_count (), DNA_BAD_ARGUMENT);

    /*
     * Remove the ISR from the appropriate queue.
     */

    it_status = cpu_trap_mask_and_backup();
    queue = & cpu_pool . cpu[cpuid] . isr[id];
    lock_acquire (& queue -> lock);

    isr = queue_lookup (queue, interrupt_handler_inspector, handler);
    check (no_isr, isr != NULL, DNA_BAD_ARGUMENT);

    queue_extract (queue, isr);

    /*
     * If there is no more handler for the specified
     * interrupt, disable it.
     */

    if (queue -> status == 0)
    {
      if (cpuid == cpu_mp_id ())
      {
        cpu_trap_disable (id);
      }
      else
      {
        lock_acquire (& cpu_pool . cpu[cpuid] . ipi_lock);
        cpu_mp_send_ipi (cpuid, DNA_IPI_TRAP_DISABLE, (void *) id);
      }
    }

    lock_release (& queue -> lock);
    cpu_trap_restore(it_status);

    kernel_free (isr);
    return DNA_OK;
  }

  rescue (no_isr)
  {
    lock_release (& queue -> lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

