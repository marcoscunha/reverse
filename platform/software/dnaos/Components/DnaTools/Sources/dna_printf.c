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

/****f* C/dna_printf
 * SUMMARY
 * Basic kernel printf support.
 *
 * SYNOPSIS
 */

void dna_printf (const char * format, ...)

/*
 * FUNCTION
 * Print formatted text.
 *
 * SOURCE
 */

{
  bool is_long = false, is_long_long = false;
  bool is_short = false, is_char = false;
  char char_value, ascii[32], * string_value, buffer[256];
  int32_t ascii_index, string_length, j = 0;
  int64_t signed_value;
  uint64_t unsigned_value, remainder;

  enum { NORMAL, ESCAPE, FORMAT } state = NORMAL;

  va_list arg;
  va_start (arg, format);

  for (int32_t i = 0; format[i] != '\0'; i += 1)
  {
    switch (state)
    {
      case NORMAL :
        {
          switch (format[i])
          {
            case '\\' :
              {
                state = ESCAPE;
                break;
              }

            case '%' :
              {
                is_char = false;
                is_short = false;
                is_long = false;
                is_long_long = false;
                state = FORMAT;
                break;
              }

            default :
              buffer[j] = format[i];
              j += 1;
              break;
          }

          break;
        }

      case ESCAPE :
        {
          switch (format[i])
          {
            case 'n' :
              {
                buffer[j] = '\n';
                j += 1;
                break;
              }

            case 't' :
              {
                buffer[j] = '\t';
                j += 1;
                break;
              }

            default :
              {
                buffer[j] = '\\';
                buffer[j + 1] = format[i];
                j += 2;
                break;
              }
          }

          state = NORMAL;
          break;
        }

      case FORMAT :
        {
          switch (format[i])
          {
            case 'h' :
              {
                if (! is_short)
                {
                  is_short = true;
                }
				else if (! is_char)
                {
                  is_short = false;
                  is_char = true;
                }
                else
                {
                  state = NORMAL;
                }

                break;
              }
            case 'l' :
              {
                if (! is_long)
                {
                  is_long = true;
                }
                else if (! is_long_long)
                {
                  is_long = false;
                  is_long_long = true;
                }
                else
                {
                  state = NORMAL;
                }

                break;
              }

            case 'u' :
              {
                ascii_index = 0;

                /*
                 * Check the data size
                 */

                if (is_long_long)
                {
                  unsigned_value = va_arg (arg, unsigned long long int);
                }
                else if (is_long)
                {
                  unsigned_value = va_arg (arg, unsigned long int);
                }
                else if (is_char)
                {
					 unsigned_value = (unsigned char)va_arg (arg, unsigned int);
                }
                else if (is_short)
                {
					 unsigned_value = (unsigned short)va_arg (arg, unsigned int);
                }
                else
                {
                  unsigned_value = va_arg (arg, unsigned int);
                }

                /*
                 * Compute the data
                 */

                do
                {
                  remainder = unsigned_value % 10;
                  unsigned_value = unsigned_value / 10;
                  ascii[ascii_index] = remainder + '0';
                  ascii_index += 1;
                }
                while (unsigned_value > 0);

                /*
                 * Echo the data into the buffer
                 */

                for (int32_t i = ascii_index - 1; i >= 0; i--)
                {
                  buffer[j] = ascii[i];
                  j += 1;
                }

                /*
                 * Return to NORMAL state
                 */

                state = NORMAL;
                break;
              }

            case 'd' :
              {
                ascii_index = 0;

                /*
                 * Check the data size
                 */

                if (is_long_long)
                {
                  signed_value = va_arg (arg, long long int);
                }
                else if (is_long)
                {
                  signed_value = va_arg (arg, long int);
                }
                else if (is_char)
                {
					 signed_value = (signed char)va_arg (arg, int);
                }
                else if (is_short)
                {
					 signed_value = (short)va_arg (arg, int);
                }
                else
                {
                  signed_value = va_arg (arg, int);
                }

                /*
                 * Check the data sign
                 */

                if (signed_value < 0)
                {
                  unsigned_value = -1 * signed_value;
                }
                else
                {
                  unsigned_value = signed_value;
                }

                /*
                 * Compute the data
                 */

                do
                {
                  remainder = unsigned_value % 10;
                  unsigned_value = unsigned_value / 10;
                  ascii[ascii_index] = remainder + '0';
                  ascii_index += 1;
                }
                while (unsigned_value > 0);

                /*
                 * Adjust the sign
                 */

                if (signed_value < 0)
                {
                  ascii[ascii_index ++] = '-';
                }

                /*
                 * Echo the data into the buffer
                 */

                for (int32_t i = ascii_index - 1; i >= 0; i--)
                {
                  buffer[j] = ascii[i];
                  j += 1;
                }

                /*
                 * Return to NORMAL state
                 */

                state = NORMAL;
                break;
              }

            case 'p' :
            case 'x' :
              {
                ascii_index = 0;

                /*
                 * Check the data size
                 */

                if (is_long_long)
                {
                  unsigned_value = va_arg (arg, unsigned long long int);
                }
                else if (is_long)
                {
                  unsigned_value = va_arg (arg, unsigned long int);
                }
                else if (is_char)
                {
					 unsigned_value = (unsigned char)va_arg (arg, unsigned int);
                }
                else if (is_short)
                {
					 unsigned_value = (unsigned short)va_arg (arg, unsigned int);
                }
                else
                {
                  unsigned_value = va_arg (arg, unsigned int);
                }

                /*
                 * Compute the data
                 */

                do
                {
                  remainder = unsigned_value & 0xf;
                  unsigned_value >>= 4;

                  if (remainder < 10)
                  {
                    ascii[ascii_index] = remainder + '0'; 
                    ascii_index += 1;
                  }
                  else
                  {
                    ascii[ascii_index] = remainder - 10 + 'a';
                    ascii_index += 1;
                  }
                }
                while (unsigned_value > 0);

                /*
                 * Echo the data into the buffer
                 */

                for(int32_t i = ascii_index - 1; i >= 0; i--)
                {
                  buffer[j] = ascii[i];
                  j += 1;
                }

                /*
                 * Return to NORMAL state
                 */

                state = NORMAL;
                break;
              }

            case 'c' :
              {
                unsigned_value = va_arg (arg, unsigned long int);
                char_value = unsigned_value;
                buffer[j] = char_value;
                j += 1;
                state = NORMAL;
                break;
              }

            case 's' :
              {
                string_value = va_arg (arg, char *);
                string_length = dna_strlen (string_value);

                for (int32_t i = 0; i < string_length; i ++)
                {
                  buffer[j] = string_value[i];
                  j += 1;
                }

                state = NORMAL;
                break;
              }
            case '%':
                buffer[j] = '%';
                j += 1;
                state = FORMAT;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '.':
                /* simply ignore special formatting */
                state = FORMAT;
                break;
            default :
              {
                state = NORMAL;
                break;
              }
          }

          break;
        }
    }
  }

  buffer[j] = '\0';
  platform_debug_puts (buffer);

  va_end (arg);
}

/*
 ****/

