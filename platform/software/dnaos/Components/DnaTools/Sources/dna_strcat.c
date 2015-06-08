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

char * dna_strcat (char *  dest, const char *  src)
{
  char * s1 = dest;
  const char * s2 = src;
  uint32_t count=0;

  for (; *s1 != '\0'; s1++, count++);
  for (; *s2 != '\0'; s2++, s1++,count++) *s1 = *s2;
  *s1 = '\0';
  DCACHE_FLUSH(dest,count);

  return dest;
}

