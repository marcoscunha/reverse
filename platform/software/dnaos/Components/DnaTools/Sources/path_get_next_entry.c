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


/****f* Path/path_get_next_entry
 * SUMMARY
 * Get a token from a path.
 *
 * SYNOPSIS
 */

status_t path_get_next_entry (char ** path, char * token)

/*
 * ARGUMENTS
 * * path : the original path.
 * * token : a buffer big enough to contain a file name.
 *
 * RESULT
 * * DNA_ERROR : the path is empty.
 * * DNA_NAME_TOO_LONG : sic.
 * * DNA_OK : the operation succeeded.
 *
 * SOURCE
 */

{
  char * p_start = *path, * p_end = *path;
  int32_t token_size = 0;

  watch (status_t)
  {
    ensure (*p_start != '\0', DNA_ERROR);

    /*
     * Find the first occurence of '/'
     */

    while (*p_end != '/' && *p_end != '\0')
    {
      p_end += 1;
    }

    /*
     * Copy the token
     */

    token_size = p_end - p_start;
    ensure (token_size <= DNA_FILENAME_LENGTH, DNA_NAME_TOO_LONG);

    dna_memcpy (token, p_start, token_size);
    token[token_size] = '\0';

    /*
     * Cut the token from its original string
     */

    if (*p_end == '/')
    {
      *p_end ++ = '\0';
    }
    
    *path = p_end;
    return DNA_OK;
  }
}

/*
 ****/

