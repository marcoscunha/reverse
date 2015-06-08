#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count)
{
  ASSERT_RETURN(!count,EINVAL);

  barrier -> count = 0;
  barrier -> threshold = count;

  return 0;
}
