#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "default_inline.h"
__lock_struct_t global_lock;
int global_var = 0;

#define NB_THREAD  1000
#define NB_LOOP 500
void* thread_fct(void *arg)
{
  int id = *((int*)arg);
  int j=0;
  for (j=0 ; j<NB_LOOP ; j++) {
    __lock(&global_lock, id);
    /* critical section */
    global_var++;
    __unlock(&global_lock, id);
  }
  pthread_exit(0);
}

int main()
{
  pthread_t thread[NB_THREAD];
  int       thread_id[NB_THREAD];
  int i=0;

  __initlock(&global_lock);
  for (i=0; i<NB_THREAD ; i++) {
    thread_id[i] = i;
    pthread_create(&(thread[i]), NULL, thread_fct, &(thread_id[i]));
  }

  for (i=0; i<NB_THREAD ; i++) {
    pthread_join(thread[i], NULL);
  }

  printf("global_var: %d\n", global_var);
  return 0;
}

