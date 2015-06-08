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
#include <DnaTools/DnaTools.h>

int32_t dna_atoi (char * alpha)
{
  int32_t value = 0, c = 0, index = 0;
  uint32_t len = dna_strlen (alpha), coeff = 1;

  if (alpha == NULL) return -1;
  if (len == 0) return -1;
  
  for (index = len - 1; index >= 0; index -= 1)
  {
    c = alpha[index] - '0';
    if (c < 0 || c > 9) return -1;

    value += c * coeff;
    coeff *= 10;
  }

  return value;
}

