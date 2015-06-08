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

#include <Processor/Processor.h>
#include <Platform/Platform.h>
#include <Processor/apic_regs.h>

/*
 * TODO: Remove this platform specific Header
 *       Move to Platform/Platform.h
 */
#include <PCPlatformDriver/Driver.h>

void cpu_timer_get (int32_t id, bigtime_t * value)
{
    uint64_t crt_cycles = get_cycles ();

    *value = (crt_cycles * 1000) / (cpu_cycles_per_ms / 1000);
}

