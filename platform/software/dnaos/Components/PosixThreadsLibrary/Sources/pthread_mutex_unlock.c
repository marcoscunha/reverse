#include <Private/PosixThreads.h>

int pthread_mutex_unlock (pthread_mutex_t *mutex)
{
  if (mutex == NULL) return EFAULT;

  if (mutex -> lock == -1) return EINVAL;

  mutex -> status = MUTEX_UNLOCKED;
  DCACHE_FLUSH(&mutex->status,sizeof(uint32_t));
  semaphore_release (mutex -> lock, 1, 0);

  return 0;
}
