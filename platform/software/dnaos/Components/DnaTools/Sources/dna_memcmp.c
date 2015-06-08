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

#include <stdint.h>
#include <DnaTools/DnaTools.h>

int32_t dna_memcmp (const void *s1, const void *s2, size_t n)
{
  for (size_t i = 0; i < n; i += 1)
  {
    if (((uint8_t *)s1)[i] != ((uint8_t *)s2)[i])
    {
      if (((uint8_t *)s2)[i] > ((uint8_t *)s1)[i])
      {
        return (int32_t)(((uint8_t *)s2)[i] - ((uint8_t *)s1)[i]);
      }
      else
      {
        return (int32_t)(((uint8_t *)s1)[i] - ((uint8_t *)s2)[i]);
      }
    }
  }
  
  return 0;
}

