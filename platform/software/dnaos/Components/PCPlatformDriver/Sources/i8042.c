/* Intel 8042 - Keyboard & Mouse Controller */

#include <Private/Driver.h>

#include "i8042.h"

unsigned char i8042_kbd_read(void) {
   return inb(I8042_DATA_PORT);
}

