#include <Private/PosixThreads.h>
#include <Processor/Cache.h>

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
  status_t status = DNA_OK;

  if (mutex->lock == -1)
  {
    status = semaphore_create ("pthread_mutex", 1, & mutex -> lock);
    if (status != DNA_OK) return EINVAL;
  }
  semaphore_acquire (mutex -> lock, 1, 0, 0);

  mutex -> status = MUTEX_LOCKED;
  DCACHE_FLUSH(&mutex->status,sizeof(uint32_t));

  return 0;
}
