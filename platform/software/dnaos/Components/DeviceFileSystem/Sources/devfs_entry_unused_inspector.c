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

#include <Private/DeviceFileSystem.h>
#include <DnaTools/DnaTools.h>

bool devfs_entry_unused_inspector (void * entry, va_list list)
{
  bool is_dot, is_dotdot;
  devfs_entry_t e = entry;

  watch (bool)
  {
    ensure (e != NULL, false);

    is_dot = dna_strcmp (e -> name, ".") == 0;
    is_dotdot = dna_strcmp (e -> name, "..") == 0;

    return  ! is_dot && ! is_dotdot && ! e -> loaded;
  }
}

