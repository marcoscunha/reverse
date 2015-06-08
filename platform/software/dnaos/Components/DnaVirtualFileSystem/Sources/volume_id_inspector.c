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

#include <Private/VirtualFileSystem.h>
#include <DnaTools/DnaTools.h>

/****f* VolumePrivate/volume_id_inspector
 * SUMMARY
 * Compare a string and a vnode's absolute path.
 *
 * SYNOPSIS
 */

bool volume_id_inspector (void * ns, va_list list)

/*
 * ARGUMENTS
 * * ns : the ns element.
 * * id : an id.
 *
 * FUNCTION
 * Compare id and ns -> name.
 *
 * RESULT
 * Return TRUE if the ID match, FALSE otherwise.
 *
 * SOURCE
 */

{
  volume_t nspace = ns;
  int32_t nid = va_arg (list, int32_t);

  watch (bool)
  {
    ensure (nspace != NULL, false);
    return nspace -> id == nid;
  }
}

/*
 ****/

