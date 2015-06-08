#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_barrier_wait(pthread_barrier_t *barrier)
{
  int res = 0;
#if 0
  thread_t t_self = core_thread_self (), to_wake = NULL;

  ASSERT_RETURN(barrier == NULL,EINVAL);

  lock_acquire(barrier -> sem);

  barrier -> count++;
  if (barrier -> count < barrier -> threshold) {
    queue_add ((queue_t *) & (barrier -> waiting_queue), & t_self -> status_link);

    core_suspend (barrier -> sem);
    lock_acquire(barrier -> sem);
  }
  else {
    /*---- TODO: The locking scheme can be optimized ----*/
    to_wake = queue_rem ((queue_t *) & (barrier -> waiting_queue));
    do core_resume (to_wake);
    while ((to_wake = queue_rem ((queue_t *) & (barrier -> waiting_queue))) != NULL);
      
    barrier -> count = 0;
    res = PTHREAD_BARRIER_SERIAL_THREAD;
  }

  lock_release(barrier -> sem);
#endif
  return res;
}
