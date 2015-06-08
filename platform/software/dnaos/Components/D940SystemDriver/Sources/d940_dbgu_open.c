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

#include <Private/DBGU.h>
#include <Private/Driver.h>
#include <DnaTools/DnaTools.h>

status_t d940_dbgu_open (char * name, int32_t mode, void ** data)
{
  if (dna_strcmp (name, "serial/d940/0") == 0
      || dna_strcmp (name, "serial/d940/1") == 0
      || dna_strcmp (name, "serial/kernel/debug") == 0
      || dna_strcmp (name, "serial/kernel/console") == 0)
  {
    *data = & d940_dbgu_driver;
    return DNA_OK;
  }

  return DNA_ERROR;
}

