#include <Private/PosixThreads.h>

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
  if (mutex == NULL) return EFAULT;
  if (mutex -> lock == -1) return EINVAL;

  switch (semaphore_acquire (mutex -> lock, 1, DNA_RELATIVE_TIMEOUT, 0))
  {
    case DNA_WOULD_BLOCK :
      return EBUSY;

    case DNA_OK :
      mutex -> status = MUTEX_LOCKED;
      return 0;

    default :
      return EINVAL;
  }

  return 0;
}
