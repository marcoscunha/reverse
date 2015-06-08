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

#include <Private/Driver.h>
#include <DnaTools/DnaTools.h>

#include <stdio.h>
#include "i8042.h"

static int row = 0;

extern void vbe_putc(char c);

const char keymap[256] = {
  ' ', ' ', '&', ' ', '\"', '\'', '(', '-', ' ', '_', ' ', ' ', ')', '=', ' ', ' ',
  'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', ' ', ' ', 0x0d, ' ', 'q', 's',
  'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', ' ', ' ', ' ', ' ', 'w', 'x', 'c', 'v',
  'b', 'n', ',', ';', ':', '!', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', '<', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', '&', ' ', '\"', '\'', '(', '-', ' ', '_', ' ', ' ', ')', '=', ' ', ' ',
  'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', ' ', ' ', 0x0d, ' ', 'q', 's',
  'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', ' ', ' ', ' ', ' ', 'w', 'x', 'c', 'v',
  'b', 'n', ',', ';', ':', '!', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', '<', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
};

const char shifted_keymap[256] = {
  ' ', ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', ' ', '+', ' ', ' ',
  'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', ' ', ' ', 0x0d, ' ', 'Q', 'S',
  'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', ' ', ' ', ' ', ' ', 'W', 'X', 'C', 'V',
  'B', 'N', '?', '.', '/', '§', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', '>', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', ' ', '+', ' ', ' ',
  'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', ' ', ' ', 0x0d, ' ', 'Q', 'S',
  'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', ' ', ' ', ' ', ' ', 'W', 'X', 'C', 'V',
  'B', 'N', '?', '.', '/', '§', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', '>', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
};

static unsigned char shift_pressed = 0;

int32_t pc_tty_isr (void * data __attribute__((unused)))
{
  unsigned char scancode = i8042_kbd_read();

  switch(scancode) {
    case 0x2a:
    case 0x36:
      //printf("shift pressed\n");
      shift_pressed = 1;
      return;
    case 0xaa:
    case 0xb6:
      //printf("shift released\n");
      shift_pressed = 0;
      return;
  }

  char c = shift_pressed ? shifted_keymap[scancode] : keymap[scancode];
  if(c == 0) return;
  if((scancode & 0x80) == 0x80) return; // ignore key release

#if 0
  char message[128];
  snprintf(message, sizeof(message), "pc_tty_isr: scancode: 0x%02x, char:%c\n", scancode, c); 
  message[sizeof(message) - 1] = 0;
  dna_printf(message);

  unsigned char *ptr = message;
  while(*ptr) vbe_putc(*ptr++);
#endif

  vbe_putc(c);

  pc_tty_t *tty = (pc_tty_t *) &TTY[0]; 
   
  if (tty->buffer.empty) {
    tty->buffer.empty = false;
  }
  tty->buffer.data = c;
  semaphore_release(tty->sem_id, 1, DNA_NO_RESCHEDULE);
  return DNA_INVOKE_SCHEDULER;
}

