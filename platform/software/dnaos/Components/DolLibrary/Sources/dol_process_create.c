#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <Private/DolLibrary.h>
#include <Core/Core.h>

dol_status_t dol_process_create (DOLProcess * p, char * name,
    int32_t inports, int32_t outports, int32_t n, ...)
{
  va_list dimensions;
  dol_process_info_t * info = NULL;
  thread_info_t thread_info = DNA_THREAD_DEFAULTS;

  /*
   * Allocate the process memory and its ports.
   */

  info = malloc (sizeof (struct _dol_process_info));
  memset (info, 0, sizeof (struct _dol_process_info));

  if (inports != 0)
  {
    info -> n_inputs = inports;
    info -> inputs = malloc (inports * sizeof (dol_port_t));
    memset (info -> inputs, 0, sizeof (dol_port_t));
  }

  if (outports != 0)
  {
    info -> n_outputs = outports;
    info -> outputs = malloc (outports * sizeof (dol_port_t));
    memset (info -> outputs, 0, sizeof (dol_port_t));
  }

  /*
   * Build the process' index.
   */

  info -> index . n = n;
  va_start (dimensions, n);

  if (n >= 1) info -> index . x = va_arg (dimensions, int32_t);
  if (n >= 2) info -> index . y = va_arg (dimensions, int32_t);
  if (n == 3) info -> index . z = va_arg (dimensions, int32_t);

  va_end (dimensions);

  /*
   * Create the process' thread.
   */

  strcpy (thread_info . name, name);
  thread_create (dol_process_bootstrap, p, thread_info, & info -> thread_id);

  /*
   * Fill-in the process WPTR.
   */

  p -> wptr = (void *) info;
  return DOL_OK;
}

