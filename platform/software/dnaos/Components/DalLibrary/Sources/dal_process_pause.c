#include <Private/DalLibrary.h>
#include <Core/Core.h>

void dal_process_pause(DALProcess* p) { 
  dal_process_info_t * info = p -> wptr;
  thread_suspend(info->thread_id);
}
