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
#include <Processor/Processor.h>

status_t soclib_fb_write (void * handler, void * source,
    int64_t offset, int32_t * p_count)
{
  soclib_framebuffer_t * fb = handler;
  uint8_t * destination = NULL;

  if (fb -> autorewind)
  {
    offset = offset % (fb -> config . width * fb -> config . height * 2);
  }

  destination = fb -> config . buffer + offset;

  dna_log(VERBOSE_LEVEL, "FB: %d bytes @ 0x%x offset to 0x%x",
      *p_count, (int32_t)offset, destination);

  cpu_vector_write (UINT8, destination, source, *p_count);
  return DNA_OK;
}

