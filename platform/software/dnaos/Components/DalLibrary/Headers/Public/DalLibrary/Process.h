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

#ifndef DAL_PROCESS_H
#define DAL_PROCESS_H

#include <stdbool.h>
#include <stdint.h>
#include <DalLibrary/Status.h>

struct _dal_process;

typedef struct _local_states *LocalState;
typedef void (* ProcessPreinit) (struct _dal_process *);
typedef void (* ProcessInit) (struct _dal_process *);
typedef int  (* ProcessFire) (struct _dal_process *);
typedef void (* ProcessFinish) (struct _dal_process *);
typedef void * WPTR;

typedef struct _dal_process {
  LocalState      local;
  ProcessPreinit  preinit;
  ProcessInit     init;
  ProcessFire     fire;
  ProcessFinish   finish;
  WPTR wptr;

  int appID;
  int processID;
  int location;
  int *index;
} DALProcess;

#define DAL_detach(p)                           \
     {                                          \
          dal_process_cancel (p);               \
          return 0;                             \
     }

extern dal_status_t dal_process_create (DALProcess * p, char * name,  int32_t inports, int32_t outports);
extern int32_t dal_process_get_index (DALProcess * p, int32_t dim);
extern void dal_process_start (DALProcess * p);
extern void dal_process_pause (DALProcess * p);
extern void dal_process_cancel (DALProcess * p);

void dal_process_join(void);

#endif
