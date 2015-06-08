#include <unistd.h>
#include <DolLibrary/DolLibrary.h>

int32_t DOL_write (void * port, void * data, int32_t n, DOLProcess *process)
{
  int16_t fd = (int32_t) port;
  return write (fd, data, n);
}
