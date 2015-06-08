#include <Private/PosixThreads.h>

int pthread_cond_destroy(pthread_cond_t *cond)
{
  semaphore_destroy (cond -> semaphore);
  return 0;
}
