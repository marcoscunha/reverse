#include <Platform/Platform.h>
#include <Processor/IO.h>

void platform_debug_puts (char * string)
{
  char * p = string;

  while (*p != '\0')
  {
    cpu_write (UINT8, PLATFORM_DEBUG_CHARPORT, *p++);
  }
}

