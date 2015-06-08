/*  > Disclaimer:
 *  Copyright (C) 2007 - TIMA Laboratory
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License aint32_t with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdint.h>
#include <stdbool.h>
#include <DnaTools/DnaTools.h>

int32_t dna_itoa (int32_t integer, char * buffer)
{
  char ascii[64];
  int32_t value = integer, index = 0, rem = 0;
  bool negative = false;

  if(value < 0)
  {
    value = -1 * value;
    negative = true;
  }

  do
  {
    rem = value % 10;
    value = value / 10;
    ascii[index ++] = rem + '0';
  }
  while(value > 0);

  if (negative) ascii[index ++] = '-';

  for(int32_t i = index - 1; i >= 0; i--) *buffer ++ = ascii[i];
  *buffer = '\0';

  return integer;
}

