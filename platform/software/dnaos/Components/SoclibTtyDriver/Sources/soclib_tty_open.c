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

#include <Private/Soclib.h>
#include <Processor/Processor.h>
#include <DnaTools/DnaTools.h>

status_t soclib_tty_open (char * name, int32_t mode, void ** data)
{
  watch (status_t)
  {
    ensure (data != NULL, DNA_ERROR);

    if (dna_strcmp (name, "0") == 0 || dna_strcmp (name, "debug") == 0)
    {
      *data = (void *) & TTY[0];
      return DNA_OK;
    }
    else if (dna_strcmp (name, "1") == 0 || dna_strcmp (name, "console") == 0)
    {
      *data = (void *) & TTY[1];
      return DNA_OK;
    }

    return DNA_ERROR;
  }
}

