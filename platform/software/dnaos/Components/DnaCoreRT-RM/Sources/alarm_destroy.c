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

/****f* Alarm/alarm_destroy
 * SUMMARY
 * Destroy an existing alarm.
 *
 * SYNOPSIS
 */

status_t alarm_destroy (int32_t aid)

/*
 * ARGUMENTS
 * * aid : the alarm's id to cancel.
 *
 * RESULT
 * * DNA_UNKNOWN_ALARM: the alarm doesn't exist
 * * DNA_OK: the operation succeeded
 *
 * SOURCE
 */

{
  alarm_t alarm = NULL;
  alarm_id_t alarm_id = { .raw = aid };
  status_t status = DNA_OK;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    it_status = cpu_trap_mask_and_backup();
    lock_acquire (& alarm_manager . lock);

    alarm = alarm_manager . alarm[alarm_id . s . index];
    check (alarm_error, alarm != NULL, DNA_UNKNOWN_ALARM);
    check (alarm_error, alarm -> id . raw == alarm_id . raw, DNA_UNKNOWN_ALARM);

    lock_acquire (& alarm -> lock);
    lock_release (& alarm_manager . lock);

    alarm -> is_invalid = true;
    lock_release (& alarm -> lock);

    cpu_trap_restore(it_status);
    return status;
  }

  rescue (alarm_error)
  {
    lock_release (& alarm_manager . lock);
    cpu_trap_restore(it_status);
    leave;
  }
}

/*
 ****/

