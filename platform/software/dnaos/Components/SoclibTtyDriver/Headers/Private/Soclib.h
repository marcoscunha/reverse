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

#ifndef SYSTEM_SOCLIB_H
#define SYSTEM_SOCLIB_H

#include <Private/Tty.h>
#include <DnaTools/DnaTools.h>

extern status_t soclib_init_hardware (void);
extern status_t soclib_init_driver (void);
extern void soclib_uninit_driver (void);
extern const char ** soclib_publish_devices (void);
extern device_cmd_t * soclib_find_device (const char * name);

#endif
