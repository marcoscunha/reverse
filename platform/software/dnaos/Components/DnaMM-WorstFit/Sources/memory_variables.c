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

#include <Private/MemoryManager.h>
#include <DnaTools/DnaTools.h>

/****v* framework/memory_component
 * SUMMARY
 * Instance of the memory component.
 *
 * SYNOPSIS
 */

component_t memory_component =
{
  .name = "memory_component",
  .create = memory_create,
  .destroy = memory_destroy,
  .start = memory_start,
  .stop = memory_stop
};

/*
 *****/

/****v* memory_base/kernel_allocator
 * SUMMARY
 * Instance of the system's kernel allocator.
 *
 * SYNOPSIS
 */

kernel_allocator_t kernel_allocator;

/*
 *****/

