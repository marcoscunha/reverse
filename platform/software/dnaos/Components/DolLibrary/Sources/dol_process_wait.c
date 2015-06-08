#include <Private/DolLibrary.h>
#include <Core/Core.h>

void dol_process_wait (DOLProcess* p)
{ 
  dol_process_info_t * info = p -> wptr;
  thread_wait (info -> thread_id, NULL);
}
