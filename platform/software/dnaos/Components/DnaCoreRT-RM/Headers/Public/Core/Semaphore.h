/****h* Core/Semaphore
 * SUMMARY
 * Semaphore management.
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

#ifndef DNA_CORE_SEMAPHORE_PUBLIC_H
#define DNA_CORE_SEMAPHORE_PUBLIC_H

#include <Core/Alarm.h>
#include <DnaTools/DnaTools.h>

/****t* Semaphore/DNA_NO_RESCHEDULE
 * SUMMARY
 * DNA_NO_RESCHEDULE mode value.
 *
 * SOURCE
 */

#define DNA_NO_RESCHEDULE       0x00000001

/*
 ****/

/****t* Semaphore/DNA_RELATIVE_TIMEOUT
 * SUMMARY
 * DNA_RELATIVE_TIMEOUT mode value.
 *
 * SOURCE
 */

#define DNA_RELATIVE_TIMEOUT    0x00000002

/*
 ****/

/****t* Semaphore/DNA_ABSOLUTE_TIMEOUT
 * SUMMARY
 * DNA_ABSOLUTE_TIMEOUT mode value.
 *
 * SOURCE
 */

#define DNA_ABSOLUTE_TIMEOUT    0x00000004

/*
 ****/

/****t* Semaphore/DNA_CAN_INTERRUPT
 * SUMMARY
 * DNA_CAN_INTERRUPT mode value.
 *
 * SOURCE
 */

#define DNA_CAN_INTERRUPT       0x00000008

/*
 ****/

/****t* Semaphore/semaphore_info_t
 * SUMMARY
 * Semaphore information type.
 *
 * SOURCE
 */

typedef struct _semaphore_info
{
  char name[DNA_NAME_LENGTH];
  int32_t tokens;
  int32_t latest_holder;
}
semaphore_info_t;

/*
 ****/

extern status_t semaphore_create (char * name, int32_t tokens, int32_t * id);
extern status_t semaphore_destroy (int32_t id);

extern status_t semaphore_acquire (int32_t id, int32_t tokens,
    int32_t flags, bigtime_t timeout);
extern status_t semaphore_release (int32_t id, int32_t tokens, int32_t flags);

extern status_t semaphore_get_info (int32_t id, semaphore_info_t * info);

#endif

