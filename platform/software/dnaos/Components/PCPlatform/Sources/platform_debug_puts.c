#include <Platform/Platform.h>
#include <string.h>

void vga_crt_puts (char * string, int cnt);

void platform_debug_puts (char * string)
{
    vga_crt_puts (string, strlen (string));
}

