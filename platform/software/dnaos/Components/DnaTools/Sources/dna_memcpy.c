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

#include <stdlib.h>
#include <DnaTools/DnaTools.h>
#include <Processor/Cache.h>


void * dna_memcpy (void *  s1, const void *  s2, size_t n)
{
  for (size_t idx = 0; idx < n; idx += 1)
  {    
//    DCACHE_INVAL_LOST(&(((uint8_t*)s2)[idx]));
    ((uint8_t *)s1)[idx] = ((uint8_t *)s2)[idx];
//    DCACHE_FLUSH_FAST(&(((uint8_t*)s1)[idx]));
  }
  DCACHE_FLUSH(s1, n);
  DCACHE_INVAL(s1, n);

  return s1;
}

