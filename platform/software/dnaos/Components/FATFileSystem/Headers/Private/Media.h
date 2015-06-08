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
 
#ifndef MEDIA_H
#define MEDIA_H

extern int media_open(const char *dev_path, int32_t *fs_fd);

extern int media_read(const uint32_t sector, unsigned char *buffer, const int32_t fs_fd);

extern int media_write(const uint32_t sector, unsigned char *buffer, const int32_t fs_fd);

#endif

