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
 *
 * Adapted from :RamDisk version 6.00 by Earl Colby Pottinger.
 */

#ifndef SOCLIB_FRAMEBUFFER_PUBLIC_H
#define SOCLIB_FRAMEBUFFER_PUBLIC_H

#include <DnaTools/DnaTools.h>

enum framebuffer_requests
{
  FB_SET_AUTOREWIND = DNA_CONTROL_CODES_END
};

extern driver_t soclib_fb_module;

#endif

