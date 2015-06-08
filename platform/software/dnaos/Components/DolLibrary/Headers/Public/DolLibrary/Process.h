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

#ifndef DOL_PROCESS_H
#define DOL_PROCESS_H

#include <stdbool.h>
#include <stdint.h>
#include <DolLibrary/Status.h>

struct _dol_process;

typedef void * LocalState;
typedef void (* ProcessInit) (struct _dol_process *);
typedef int (* ProcessFire) (struct _dol_process *);
typedef void * WPTR;

typedef struct _dol_process
{
	LocalState local;
  ProcessInit init;
  ProcessFire fire;
	WPTR wptr;
}
DOLProcess;

#define GETINDEX(dim) dol_pocess_get_index (p,dim)	

#define DOL_detach(p)           \
{                               \
  dol_process_cancel (p);       \
  return 0;                     \
}

extern dol_status_t dol_process_create (DOLProcess * p, char * name,
    int32_t inports, int32_t outports, int32_t n, ...);
extern int32_t dol_process_get_index (DOLProcess * p, int32_t dim);

extern void dol_process_start (DOLProcess * p);
extern void dol_process_wait (DOLProcess * p);
extern void dol_process_cancel (DOLProcess * p);

#endif
