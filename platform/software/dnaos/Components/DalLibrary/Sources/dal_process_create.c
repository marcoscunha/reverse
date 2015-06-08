#include <string.h>
#include <Private/DalLibrary.h>
#include <Core/Core.h>
#include <MemoryManager/MemoryManager.h>

#include <stdio.h>

static int __dal_init_done = 0;

static void dal_init(void) {
  semaphore_create("dal_process_sem", 0, &__dal_sem);

  while(semaphore_acquire(__dal_sem, 1, DNA_RELATIVE_TIMEOUT, 0) == DNA_OK);
}

dal_status_t dal_process_create (DALProcess * p, char * name, int32_t inports, int32_t outports) {
  dal_process_info_t * info = NULL;
  thread_info_t thread_info;

  /*
   * Allocate the process memory and its ports.
   */

  info = kernel_malloc (sizeof (struct _dal_process_info), false);
  if(info == NULL){
    dna_printf("panic: error malloc'ing DAL process\n");
    while(1);
  }
  memset (info, 0, sizeof (struct _dal_process_info));

  /*
   * Create the process' thread.
   */

  DNA_THREAD_SET_DEFAULTS(thread_info);
  strcpy (thread_info . name, name);
  int res = thread_create (dal_process_bootstrap, p, thread_info, & info -> thread_id);
  if(res != DNA_OK) {
    printf("panic: error creating thread %s, res=%d\n", name, res);
    while(1);
  }
  if(info->thread_id == 0){
    printf("panic: error creating thread %s, res=%d\n", name, res);
    while(1);
  }

  /*
   * Fill-in the process WPTR.
   */

  p -> wptr = (void *) info;

  // TODO: not thread safe
  if(!__dal_init_done) {
    dal_init();
  }
  __dal_nb_alive_processes++;  
  
  return DAL_OK;
}

