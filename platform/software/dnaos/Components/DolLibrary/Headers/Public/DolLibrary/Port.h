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

#ifndef DOL_PORT_H
#define DOL_PORT_H

#include <stdint.h>
#include <stdarg.h>
#include <DolLibrary/Process.h>
#include <DolLibrary/Status.h>

typedef enum _dol_port_direction
{
  DOL_IN_PORT,
  DOL_OUT_PORT
}
dol_port_type_t;

#define CREATEPORTVAR(port) int32_t port

#define CREATEPORT(port, base, n, ...) \
  dol_port_get (p, & port, base, n, ## __VA_ARGS__);

extern dol_status_t dol_port_get (DOLProcess * p, int32_t * port,
    char * name, int32_t n, ...); 

extern dol_status_t dol_port_init (DOLProcess * p, char * name,
    dol_port_type_t type, char * path, int32_t n, ...);  

extern int32_t DOL_read (void * port, void * data, int32_t n, DOLProcess * p);
extern int32_t DOL_write (void * port, void * data, int32_t n, DOLProcess * p);

#endif
