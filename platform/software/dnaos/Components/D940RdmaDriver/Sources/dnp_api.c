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

#include <Private/dnp_api.h>


// Send a buffer description for registration
void dnp_send_lut_entry(dnp_lut_entry_t *lutentry, uint32_t lut_id)
{
    dnp_lut_entry_t *ptr = (dnp_lut_entry_t *)(AHB_OFFSET +  DNP_LUT_START_OFFSET + lut_id * DNP_LUT_ENTRY_SIZE  ) ;

    cpu_vector_write(UINT32, ptr, lutentry, sizeof(dnp_lut_entry_t) / 4);
}

// Read caracteristics of a registerd buffer
void dnp_read_lut_entry(dnp_lut_entry_t *lutentry, uint32_t lut_id)
{
    dnp_lut_entry_t *ptr = (dnp_lut_entry_t *)(AHB_OFFSET +  DNP_LUT_START_OFFSET + lut_id * DNP_LUT_ENTRY_SIZE  ) ;

    cpu_vector_read(UINT32, lutentry, ptr, sizeof(dnp_lut_entry_t) / 4);
}


// Send a command to the slave interface of the dnp
void dnp_send_cmd(dnp_command_t *cmd)
{
    dnp_command_t *ptr = (dnp_command_t *)(AHB_OFFSET + DNP_COMMAND_START_OFFSET) ;

    cpu_vector_write(UINT32, ptr, cmd, sizeof(dnp_command_t) / 4);
}


/*
 *  Note: for the moment, in dnp_send_cmd, we always send on the first slot of the
 *        DNP's command FIFO
 */
