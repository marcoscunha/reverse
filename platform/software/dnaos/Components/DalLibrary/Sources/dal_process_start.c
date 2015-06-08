#include <Private/DalLibrary.h>
#include <Core/Core.h>

void dal_process_start (DALProcess* p)
{ 
  dal_process_info_t * info = p -> wptr;
  thread_resume (info -> thread_id);
}
