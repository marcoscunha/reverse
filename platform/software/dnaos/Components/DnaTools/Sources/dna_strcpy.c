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

#include <DnaTools/DnaTools.h>
#include <Processor/Cache.h>

inline char * dna_strcpy (char *  s1, const char *  s2)
{
  char *s1_ptr = s1, *s2_ptr = (char *)s2;
  uint32_t count=0;
 
  do{
      *s1_ptr++ = *s2_ptr;
      count++;
  }while (*s2_ptr++ != '\0');

  DCACHE_FLUSH(s1,count);
  DCACHE_INVAL(s1,count);
 
  return s1;
}

