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

#ifndef MASTERBOOTRECORD_PARTITION_H
#define MASTERBOOTRECORD_PARTITION_H

#include <stdint.h>

typedef struct _mbr_partition
{
  uint8_t status;
  uint8_t head_first;
  uint8_t sector_first;
  uint8_t cylinder_first;
  uint8_t type;
  uint8_t head_last;
  uint8_t sector_last;
  uint8_t cylinder_last;
  uint32_t block_start;
  uint32_t block_count;
}
mbr_partition_t;

#endif
