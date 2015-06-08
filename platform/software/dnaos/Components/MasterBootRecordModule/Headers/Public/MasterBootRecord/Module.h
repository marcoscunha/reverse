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

#ifndef MASTERBOOTRECORD_MODULE_H
#define MASTERBOOTRECORD_MODULE_H

#include <stdint.h>
#include <MasterBootRecord/Partition.h>
#include <DnaTools/DnaTools.h>

typedef struct _mbr_extension
{
  extension_t extension;
  status_t (* load) (uint8_t sector[512], mbr_partition_t partition[4]);
  status_t (* dump) (mbr_partition_t partition[4], uint8_t sector[512]);
}
mbr_extension_t;

#endif

