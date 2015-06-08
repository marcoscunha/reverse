#include <Private/DolLibrary.h>

int32_t dol_process_bootstrap (void * data)
{
  DOLProcess * p = (DOLProcess *)data;
  dol_process_info_t * info = p -> wptr;

  /*
   * Initialize the process.
   */

  p -> init (p);

  /*
   * Fire the process until it is canceled.
   */

  while (! info -> canceled)
  {
    p -> fire (p);
  }

  return 0;
}
