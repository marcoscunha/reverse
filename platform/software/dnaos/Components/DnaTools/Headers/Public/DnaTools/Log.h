/****h* Tools/Log
 * SUMMARY
 * Log management.
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

#ifndef DNA_TOOLS_LOG_H
#define DNA_TOOLS_LOG_H

#include <DnaTools/C.h>
#include <Processor/Processor.h>

#define PANIC_LEVEL 1
#define INFO_LEVEL 2
#define VERBOSE_LEVEL 3

#define LOG(string, ...)                                      \
    dna_printf ("<%d>(%s:%d): " string "\r\n", cpu_mp_id (),  \
        __FUNCTION__, __LINE__,  ## __VA_ARGS__);

#ifndef DNA_ENABLE_LOG
#define DNA_ENABLE_LOG 0
#else
#if DNA_ENABLE_LOG < PANIC_LEVEL || DNA_ENABLE_LOG > VERBOSE_LEVEL
#error __FUNCTION ", " __LINE__ ": invalid log level"
#endif
#endif

#if (DNA_ENABLE_LOG >= PANIC_LEVEL)
#define LOG_PANIC_LEVEL(string, ...) LOG(string, ## __VA_ARGS__)
#else
#define LOG_PANIC_LEVEL(string, ...)
#endif

#if (DNA_ENABLE_LOG >= INFO_LEVEL)
#define LOG_INFO_LEVEL(string, ...) LOG(string, ## __VA_ARGS__)
#else
#define LOG_INFO_LEVEL(string, ...)
#endif

#if (DNA_ENABLE_LOG == VERBOSE_LEVEL)
#define LOG_VERBOSE_LEVEL(string, ...) LOG(string, ## __VA_ARGS__)
#else
#define LOG_VERBOSE_LEVEL(string, ...)
#endif

#define dna_log(level, string, ...) LOG_##level (string, ## __VA_ARGS__)

#endif
