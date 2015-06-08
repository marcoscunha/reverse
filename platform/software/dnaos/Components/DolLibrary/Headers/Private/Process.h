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

#ifndef DOL_PROCESS_PRIVATE_H
#define DOL_PROCESS_PRIVATE_H

#include <Private/Port.h>
#include <DolLibrary/Process.h>

typedef struct _dol_process_index
{
	int32_t n;
	int32_t x, y, z;
}
dol_process_index_t;

typedef struct _dol_process_info
{
  int32_t thread_id;
	bool canceled;
	bool is_scheduling_static;

  int32_t n_inputs;
  dol_port_t * inputs;

  int32_t n_outputs;
  dol_port_t * outputs;

	dol_process_index_t index;
}
dol_process_info_t;

extern int32_t dol_process_bootstrap (void * process);

#endif
