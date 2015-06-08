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

uint32_t dna_log2 (uint32_t value)
{
  static const uint32_t b[] =
  {
    0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000
  };

  static const uint32_t S[] =
  {
    1, 2, 4, 8, 16
  };

  register uint32_t r = 0;

  for (int32_t i = 4; i >= 0; i -= 1)
  {
    if (value & b[i])
    {
      value >>= S[i];
      r |= S[i];
    }
  }

  return r;
}

