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

#ifndef D940_MMC_DRIVER_PRIVATE_H
#define D940_MMC_DRIVER_PRIVATE_H

#include <DnaTools/DnaTools.h>

extern const char * d940_mmc_devices[];

extern status_t d940_mmc_init_hardware (void);
extern status_t d940_mmc_init_driver (void);
extern void d940_mmc_uninit_driver (void);
extern const char ** d940_mmc_publish_devices (void);
extern device_cmd_t * d940_mmc_find_device (const char * name);

#endif

