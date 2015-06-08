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

/****f* Path/path_split
 * SUMMARY
 *
 * SYNOPSIS
 */

status_t path_split (char * restrict path, char * restrict token)

/*
 * ARGUMENTS
 *
 * FUNCTION
 *
 * RESULT
 *
 * SOURCE
 */

{
  char * p_start = NULL, * p_end = NULL;
  int32_t token_size;

  watch (status_t)
  {
    ensure (path != NULL && token != NULL, DNA_ERROR);

    /*
     * Compute the end of the token. If the path
     * ends with '/', add '.' so as to return a valid '.' entry.
     */

    p_end = path + dna_strlen (path);

    if (*p_end == '/')
    {
      *(p_end + 1) = '.';
      p_end += 1;
    }

#if 0 
    /*
     * Not sure that is necessary anymore. It was here to 
     * deal with multiple consecutive slashes.
     */

    while (*p_end == '/') p_end -= 1;
    p_start = p_end;
#endif

    /*
     * Look for the beginning of the token, that is to
     * say a '/' character.
     */

    while (*p_start != '/' && p_start != path)
    {
      p_start -= 1;
    }
    ensure (*p_start == '/', DNA_ERROR);

    /*
     * Copy the token.
     */

    token_size = p_end - p_start + 1;
    ensure (token_size <= DNA_FILENAME_LENGTH, DNA_NAME_TOO_LONG);

    dna_memcpy (token, p_start + 1, token_size);
    token[token_size] = '\0';

    /*
     * Close the token into the original path.
     * I might be safe to directly close slash.
     */

#if 0
    *(p_start + 1) = '\0';
#else
    *(p_start + 1) = '\0';
#endif

    return DNA_OK;
  }
}

/*
 ****/

