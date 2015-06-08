#include <Private/PosixThreads.h>

int pthread_cond_broadcast(pthread_cond_t *condition)
{
  status_t status = DNA_OK;
  DCACHE_INVAL(condition,sizeof(pthread_cond_t));
  if (condition -> count > 0)
  {
    status = semaphore_release (condition -> semaphore,
        condition -> count, 0);
    if (status == DNA_BAD_SEM_ID) return EINVAL;
  }
  DCACHE_FLUSH(condition,sizeof(pthread_cond_t));

  return 0;
}
