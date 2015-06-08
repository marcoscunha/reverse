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

/****f* VNode/vnode_volume_inspector
 * SUMMARY
 * Compare an volume and a vnode's volume.
 *
 * SYNOPSIS
 */

bool vnode_volume_inspector (void * node, va_list list)

/*
 * ARGUMENTS
 * * node : a vnode element.
 * * p_volume : a pointer to an volume.
 *
 * FUNCTION
 * Compare *p_volume and vnode -> volume.
 *
 * RESULT
 * Return TRUE if the volumes match, FALSE otherwise.
 *
 * SOURCE
 */

{
  vnode_t vnode = node;
  volume_t volume = va_arg (list, volume_t);

  watch (bool)
  {
    ensure (vnode != NULL, false);
    ensure (volume != NULL, false);
    return vnode -> volume == volume;
  }
}

/*
 ****/
