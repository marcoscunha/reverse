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

#ifndef DOL_PORT_PRIVATE_H
#define DOL_PORT_PRIVATE_H

#include <stdint.h>
#include <DolLibrary/Port.h>

#define DOL_PORT_NAME_LENGTH 64

typedef struct _dol_port
{
  char name[DOL_PORT_NAME_LENGTH];
  dol_port_type_t type;
  int16_t fd;
}
dol_port_t;

#endif
