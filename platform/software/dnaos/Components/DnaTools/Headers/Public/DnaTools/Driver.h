/****h* Tools/Driver
 * SUMMARY
 * Driver-related definitions.
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

#ifndef DNA_TOOLS_DRIVER_H
#define DNA_TOOLS_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#include <DnaTools/Device.h>
#include <DnaTools/Status.h>

/****t* Driver/driver_t
 * SUMMARY
 * Driver type.
 *
 * SOURCE
 */

typedef struct _driver
{
  char * name;
  status_t (* init_hardware) (void);
  status_t (* init_driver) (void);
  void (* uninit_driver) (void);
  const char ** (* publish_devices) (void);
  device_cmd_t * (* find_device) (const char * name);
}
driver_t;

/*
 ****/

extern int32_t OS_N_DRIVERS;
extern driver_t * OS_DRIVERS_LIST [];

#endif

