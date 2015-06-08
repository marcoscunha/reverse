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

#include <Private/OCR.h>
#include <DnaTools/DnaTools.h>

static char * ocr_string[] = 
{
  "2.7 - 2.8 voltage", "2.8 - 2.9 voltage",
  "2.9 - 3.0 voltage", "3.0 - 3.1 voltage",
  "3.1 - 3.2 voltage", "3.2 - 3.3 voltage",
  "3.3 - 3.4 voltage", "3.4 - 3.5 voltage",
  "3.5 - 3.6 voltage", "CSS on",
  "Card ready", NULL
};

static uint32_t ocr_code[] =
{
  (1UL << 15), (1UL << 16), (1UL << 17),
  (1UL << 18), (1UL << 19), (1UL << 20),
  (1UL << 21), (1UL << 22), (1UL << 23),
  (1UL << 30), (1UL << 31),
};

void mmc_show_ocr (mmc_ocr_t ocr)
{
  dna_printf ("Raw OCR = 0x%x\r\n", ocr . raw);

  for (int32_t index = 0; ocr_string[index] != NULL; index += 1)
  {
    if ((ocr . raw & ocr_code[index]) != 0)
    {
      dna_printf ("%s\r\n", ocr_string[index]);
    }
  }
}

