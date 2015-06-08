#include <Private/DolLibrary.h>
#include <Core/Core.h>

void dol_process_cancel (DOLProcess * p)
{
  dol_process_info_t * info = p -> wptr;
  info -> canceled = true;
}
