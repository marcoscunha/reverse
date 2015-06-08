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

#include <Private/Core.h>
#include <DnaTools/DnaTools.h>

/****f* AlarmPrivate/alarm_comparator
 * SUMMARY
 * Compare (<) two alarms.
 *
 * SYNOPSIS
 */

bool alarm_comparator (void * item1, void * item2)

/*
 * ARGUMENTS
 * * item1 : a pointer to an alarm_t
 * * item2 : a pointer to an alarm_t
 *
 * RESULT
 * item2 < item1.
 *
 * SOURCE
 */

{
  alarm_t alarm1 = item1;
  alarm_t alarm2 = item2;
  
  watch (bool)
  {
    ensure (alarm1 != NULL && alarm2 != NULL, false);
    return alarm2 -> deadline < alarm1 -> deadline;
  }
}

/*
 ****/

