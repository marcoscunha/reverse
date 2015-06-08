/****h* Tools/VaCalls
 * SUMMARY
 * Variadic to va_list calls.
 ****
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

#include <stdint.h>

#ifndef DNA_TOOLS_VA_CALL_H
#define DNA_TOOLS_VA_CALL_H

#define va_call(list_name, block, ...)      \
{                                           \
  void _va_call (char * name, ...)          \
  {                                         \
    va_list list_name;                      \
    va_start (list_name, name);             \
    block;                                  \
    va_end (list_name);                     \
  }                                         \
                                            \
  _va_call (#list_name, ## __VA_ARGS__);    \
}

#endif

