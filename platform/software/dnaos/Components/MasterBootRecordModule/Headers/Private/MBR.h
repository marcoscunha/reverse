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

#ifndef MASTERBOOTRECORD_MBR_H
#define MASTERBOOTRECORD_MBR_H

#include <stdint.h>
#include <MasterBootRecord/Partition.h>

typedef union _mbr
{
  uint8_t raw[512];

  struct _mbr_fields
  {
    uint8_t code[440];
    uint32_t disk_signature;
    uint16_t zero;
    mbr_partition_t partition[4];
    uint16_t signature;
  }
  __attribute__ ((packed)) fields;
}
mbr_t;

#endif


