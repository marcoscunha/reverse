/* *************************************************************************
 * Copyright (C) 2010 TIMA Laboratory                                    *
 * Author: Alexandre CHAGOYA-GARZON
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/
#ifndef __DNP_API_h
#define __DNP_API_h


#include <Processor/Processor.h>
#include <DnaTools/DnaTools.h>
#include <Private/dnp_types.h>

// Low-level functions to exchange control or commands to the DNP

// Read/write DNP registers
#define dnp_reg_write(reg, val) cpu_write(UINT32, reg, val)
#define dnp_reg_read(reg, val) cpu_read(UINT32, reg, val)

// Send a buffer description for registration
void dnp_send_lut_entry(dnp_lut_entry_t *lutentry, uint32_t lut_id);

// Read caracteristics of a registerd buffer
void dnp_read_lut_entry(dnp_lut_entry_t *lutentry, uint32_t lut_id);

// Send a command to the slave interface of the dnp
void dnp_send_cmd(dnp_command_t *cmd);

#endif

/*
 *  Note: for the moment, in dnp_send_cmd, we always send on the first slot of the
 *        DNP's command FIFO
 */
