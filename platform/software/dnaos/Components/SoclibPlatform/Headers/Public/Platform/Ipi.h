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

#ifndef PLATFORM_IPI_H
#define PLATFORM_IPI_H

#include <stdint.h>

typedef struct _soclib_ipi_port
{
  volatile uint32_t command;
  volatile uint32_t data;
  volatile uint32_t reserved;
  volatile uint32_t reset;
}
* soclib_ipi_port_t;

extern soclib_ipi_port_t PLATFORM_IPI_BASE;

#endif

