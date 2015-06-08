#include <Private/PosixThreads.h>

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
  semaphore_destroy (mutex -> lock);
  return 0;
}
