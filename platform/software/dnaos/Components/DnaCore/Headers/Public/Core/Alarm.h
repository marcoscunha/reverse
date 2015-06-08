/****h* Core/Alarm
 * SUMMARY
 * Alarm management.
 ****
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

#ifndef DNA_CORE_ALARM_PUBLIC_H
#define DNA_CORE_ALARM_PUBLIC_H

#include <stdint.h>
#include <stdbool.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

/****t* Alarm/alarm_mode_t
 * SUMMARY
 * Alarm mode type.
 *
 * SOURCE
 */

typedef enum _alarm_mode
{
  DNA_PERIODIC_ALARM          = 0x1,
  DNA_ONE_SHOT_RELATIVE_ALARM = 0x2,
  DNA_ONE_SHOT_ABSOLUTE_ALARM = 0x4
}
alarm_mode_t;

/*
 ****/

/****t* Alarm/alarm_callback_t
 * SUMMARY
 * Alarm callback type.
 *
 * SOURCE
 */

typedef status_t (* alarm_callback_t) (void * data);

/*
 ****/

extern status_t alarm_create (bigtime_t quantum, alarm_mode_t mode,
    alarm_callback_t callback, void * data, int32_t * aid);

extern status_t alarm_destroy (int32_t aid);
extern status_t alarm_handler (void);

#endif

