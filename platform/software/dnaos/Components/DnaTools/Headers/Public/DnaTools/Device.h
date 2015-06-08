/****h* Tools/Device
 * SUMMARY
 * Device-related definitions.
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

#ifndef DNA_TOOLS_DEVICE_H
#define DNA_TOOLS_DEVICE_H

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include <DnaTools/Status.h>

/****v* Device/device_control_code
 * SUMMARY
 * Device control codes.
 */

enum _device_control_codes
{
  DNA_GET_DEVICE_SIZE = 0,
  DNA_SET_DEVICE_SIZE,
  DNA_SET_NONBLOCKING_IO,
  DNA_SET_BLOCKING_IO,
  DNA_GET_READ_STATUS,
  DNA_GET_WRITE_STATUS,
  DNA_GET_INFO,
  DNA_GET_DRIVER_FOR_DEVICE,
  DNA_GET_PARTITION_INFO,
  DNA_SET_PARTITION,
  DNA_FORMAT_DEVICE,
  DNA_EJECT_DEVICE,
  DNA_FLUSH_DRIVE_CACHE,
  DNA_GET_ETH_LINK_STATUS,
  DNA_GET_ETH_MAC,
  DNA_GET_ETH_RX_STATS,
  DNA_GET_ETH_TX_STATS,
  DNA_CONTROL_CODES_END = 0x0FFF
};

/*
 ****/

/****v* Device/device_type
 * SUMMARY
 * Device types.
 */

enum _device_type
{
  DNA_CHARACTER_DEVICE,
  DNA_DISK_DEVICE,
  DNA_WORM_DEVICE,
  DNA_CD_DEVICE,
  DNA_NETWORK_DEVICE
};

/*
 ****/

/****t* Device/partition_info_t
 * SUMMARY
 * Partition information type.
 *
 * SOURCE
 */

typedef struct _partition_info
{
  int32_t id;
  int32_t logical_block_size;
  int64_t offset;
  int64_t size;
}
partition_info_t;

/*
 ****/

/****t* Device/device_info_t
 * SUMMARY
 * Device information.
 *
 * SOURCE
 */

typedef struct _device_info
{
  int32_t type;
  uint32_t block_size;
  uint32_t block_count;
  bool removable;
  bool read_only;
  bool write_once;
}
device_info_t;

/*
 ****/

/****t* Device/device_cmd_t
 * SUMMARY
 * Device commands type.
 *
 * SOURCE
 */

typedef struct _device_cmd
{
  status_t (* open) (char * name, int32_t mode, void ** cookie);
  status_t (* close) (void * cookie);
  status_t (* free) (void * cookie);
  status_t (* read) (void * handler, void * destination,
      int64_t offset, int32_t * p_res);
  status_t (* write) (void * handler, void * source,
      int64_t offset, int32_t * p_res);
  status_t (* control) (void * handler, int32_t operation,
      va_list arguments, int32_t * p_res);
}
device_cmd_t;

/*
 ****/

#endif
