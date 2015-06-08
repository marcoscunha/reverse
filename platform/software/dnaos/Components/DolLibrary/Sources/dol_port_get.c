#include <stdlib.h>
#include <string.h>
#include <Private/DolLibrary.h>

dol_status_t dol_port_get (DOLProcess * p, int32_t * port,
    char * name, int32_t n, ...)
{
  va_list pairs;
  int32_t index, offset = 0, range = 0;
  dol_port_t * aport = NULL;
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
   * Get the right port.
   */

  for (index = 0; index < info -> n_inputs; index += 1)
  {
    aport = & info -> inputs[index];
    if (aport -> fd != 0 && strcmp (aport -> name, name) == 0) break;
  }

  if (index != info -> n_inputs)
  {
    aport = & info -> inputs[index + offset];
  }
  else
  {
    for (index = 0; index < info -> n_outputs; index += 1)
    {
      aport = & info -> outputs[index];
      if (aport -> fd != 0 && strcmp (aport -> name, name) == 0) break;
    }

    if (index == info -> n_outputs)
    {
      return DOL_ERROR;
    }
    else
    {
      aport = & info -> outputs[index + offset];
    }
  }

  *(int32_t *)port = aport -> fd;
  return DOL_OK;
}
