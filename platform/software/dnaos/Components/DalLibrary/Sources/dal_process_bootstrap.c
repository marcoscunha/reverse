#include <Private/DalLibrary.h>

#include <Core/Core.h>

int32_t dal_process_bootstrap (void *data) {
  DALProcess *p = (DALProcess *)data;
  dal_process_info_t *info = p->wptr;

  p->preinit(p);

  // Initialize the process.
  p->init(p);

  // Fire the process until it is canceled.

  while(!info->canceled) {
    if(p->fire(p) != 0) {
      break;
    }
    thread_yield(); // we got deadlocks without this line (because fire functions don't yield when there is no IO on channels)
  }

  //p->stop();

  dna_printf("DAL: RELEASING SEM\n");
  semaphore_release(__dal_sem, 1, DNA_NO_RESCHEDULE);  

  return 0;
}
