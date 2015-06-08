#include <Private/PosixThreads.h>
#include <Processor/Cache.h>

int pthread_cond_wait (pthread_cond_t *condition, pthread_mutex_t *mutex)
{
  status_t status = DNA_OK;

  DCACHE_INVAL(&condition->count,sizeof(uint32_t)); // TODO: WARN#1
  status = semaphore_acquire (condition -> semaphore, 1, DNA_RELATIVE_TIMEOUT, 0);
  if (status == DNA_WOULD_BLOCK)
  {
    DCACHE_INVAL(&condition->count,sizeof(uint32_t));
    condition -> count += 1;
    DCACHE_FLUSH(&condition->count,sizeof(uint32_t));
    pthread_mutex_unlock (mutex);

    status = semaphore_acquire (condition -> semaphore, 1, 0, -1);
    pthread_mutex_lock (mutex);
    DCACHE_INVAL(&condition->count,sizeof(uint32_t));
    condition -> count -= 1;
    DCACHE_FLUSH(&condition->count,sizeof(uint32_t));
  }

  return 0;
}

