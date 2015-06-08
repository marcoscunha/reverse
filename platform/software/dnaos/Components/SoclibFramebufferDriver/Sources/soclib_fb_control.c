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

#include <Private/Driver.h>
#include <DnaTools/DnaTools.h>

status_t soclib_fb_control (void * handler, int32_t function,
    va_list arguments, int32_t * p_ret)
{
  soclib_framebuffer_t * fb = (soclib_framebuffer_t *) handler;
  bool value = va_arg(arguments, int32_t);

  switch (function)
  {
    case FB_SET_AUTOREWIND:
      dna_log(INFO_LEVEL, "Setting autorewind to 0x%x", value);

      fb -> autorewind = value;
      *p_ret = 0;
      break;

    default:
      return DNA_ERROR;
  }

  return DNA_OK;
}

