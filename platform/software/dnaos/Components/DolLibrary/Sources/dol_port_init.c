#include <string.h>
#include <fcntl.h>
#include <Private/DolLibrary.h>

dol_status_t dol_port_init (DOLProcess * p, char * name,
    dol_port_type_t type, char * path, int32_t n, ...)
{
  va_list pairs;
  int32_t index, array_size = 0, offset = 0, range = 0;
  dol_port_t * port = NULL, * port_array = NULL;
  dol_process_info_t * info = p -> wptr;

  /*
   * Compute the port offset.
   */

  va_start (pairs, n);

  if (n >= 1)
  {
    offset += va_arg (pairs, int32_t);
    range = va_arg (pairs, int32_t);
  }

  if (n >= 2)
  {
    offset += va_arg (pairs, int32_t) * range;
    range = va_arg (pairs, int32_t);
  }

  if (n == 3)
  {
    offset += va_arg (pairs, int32_t) * range;
    range = va_arg (pairs, int32_t);
  }

  va_end (pairs);

  /*
   * Get the right port array.
   */

  switch (type)
  {
    case DOL_IN_PORT :
      {
        port_array = info -> inputs;
        array_size = info -> n_inputs;
        break;
      }

    case DOL_OUT_PORT :
      {
        port_array = info -> outputs;
        array_size = info -> n_outputs;
        break;
      }
  }

  /*
   * Get the right port.
   */

  if (offset == 0)
  {
    for (index = 0; index < array_size; index += 1)
    {
      if (port_array[index] . fd == 0) break;
    }

    if (index == array_size) return DOL_ERROR;

    port = & port_array[index];
  }
  else
  {
    for (index = 0; index < array_size; index += 1)
    {
      port = & port_array[index];
      if (port -> fd != 0 && strcmp (port -> name, name) == 0) break;
    }

    if (index == array_size) return DOL_ERROR;

    port = & port_array[index + offset];
  }

  /*
   * Fill-in port information and open its device.
   */

  strcpy (port -> name, name);
  port -> type = type;
  port -> fd = open (path, O_RDWR);

  /*
   * Check if the FD is valid and return.
   */

  if (port -> fd == -1)
  {
    port -> fd = 0;
    return DOL_ERROR;
  }

  return DOL_OK;
}

