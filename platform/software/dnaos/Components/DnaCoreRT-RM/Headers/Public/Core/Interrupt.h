/****h* Core/Interrupt
 * SUMMARY
 * Interrupt management.
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

#ifndef DNA_CORE_INTERRUPT_PUBLIC_H
#define DNA_CORE_INTERRUPT_PUBLIC_H

#include <stdint.h>
#include <stdbool.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Processor.h>

extern status_t interrupt_attach (int32_t cpuid, interrupt_id_t id,
    int32_t mode, interrupt_handler_t handler, bool bypass_demux);

extern status_t interrupt_detach (int32_t cpuid, interrupt_id_t id,
    interrupt_handler_t handler);

extern status_t ipi_handler (int32_t command, void * cookie);

#endif

