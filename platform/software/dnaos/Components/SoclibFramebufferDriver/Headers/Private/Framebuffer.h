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

#ifndef SOCLIB_FRAMEBUFFER_H
#define SOCLIB_FRAMEBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <Core/Core.h>
#include <DnaTools/DnaTools.h>

typedef struct soclib_framebuffer_config
{
  uint32_t width;
  uint32_t height;
  uint8_t * buffer;
}
soclib_framebuffer_config_t;

typedef struct soclib_framebuffer
{
  char name[DNA_NAME_LENGTH];
  soclib_framebuffer_config_t config;
  bool autorewind;
}
soclib_framebuffer_t;

extern soclib_framebuffer_t * FB;
extern char ** soclib_fb_devices;
extern device_cmd_t soclib_fb_commands;

extern uint32_t SOCLIB_FB_NDEV;
extern soclib_framebuffer_config_t SOCLIB_FB_DEVICES[];

extern int32_t soclib_fb_isr (int32_t itn);

extern status_t soclib_fb_open (char * name, int32_t mode, void ** data);
extern status_t soclib_fb_close (void * data);
extern status_t soclib_fb_free (void * data);

extern status_t soclib_fb_read (void * handler, void * destination,
    int64_t offset, int32_t * p_count);
extern status_t soclib_fb_write (void * handler, void * source,
    int64_t offset, int32_t * p_count);
extern status_t soclib_fb_control (void * handler, int32_t operation,
    va_list arguments, int32_t * p_res);

#endif

