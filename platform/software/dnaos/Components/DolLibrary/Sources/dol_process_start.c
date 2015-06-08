#include <Private/DolLibrary.h>
#include <Core/Core.h>

void dol_process_start (DOLProcess* p)
{ 
  dol_process_info_t * info = p -> wptr;
  thread_resume (info -> thread_id);
}
