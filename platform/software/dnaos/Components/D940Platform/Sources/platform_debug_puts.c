#include <Platform/DBGU.h>
#include <Processor/IO.h>

void platform_debug_puts (char * string)
{
  uint32_t res = 0;

  for (int32_t i = 0; string[i] != '\0'; i += 1)
  {
    do cpu_read (UINT32, & (PLATFORM_DBGU_BASE -> SR), res);
    while ((res & 0x202) != 0x202);

    cpu_write (UINT32, & (PLATFORM_DBGU_BASE -> THR), string[i]);
  }
}

