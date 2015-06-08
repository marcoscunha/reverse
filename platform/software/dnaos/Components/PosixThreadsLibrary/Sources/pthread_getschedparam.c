#include <string.h>
#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_getschedparam(pthread_t t, int *p, struct sched_param *sp)
{
  ASSERT_RETURN (!t || !sp, EINVAL);

  *p = t -> attributs -> schedpolicy;
  memcpy (sp, & t -> attributs -> schedparam, sizeof (*sp));
  return 0;
}

