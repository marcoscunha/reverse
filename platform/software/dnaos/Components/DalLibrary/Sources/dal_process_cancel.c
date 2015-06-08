#include <Private/DalLibrary.h>
#include <Core/Core.h>

void dal_process_cancel (DALProcess * p)
{
  dal_process_info_t * info = p -> wptr;
  info -> canceled = true;
}
