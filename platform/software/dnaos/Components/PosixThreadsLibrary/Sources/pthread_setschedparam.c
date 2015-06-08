#include <string.h>
#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_setschedparam(pthread_t t, int p, const struct sched_param *sp)
{
  ASSERT_RETURN(!t || !sp,EINVAL);

  // FIXME : we need to find out if
  // the thread is still in scheduled
  // => core_search ?
  // It's the same for the _getschedparam counterpart

  t -> attributs -> schedpolicy = p;
  memcpy (& t -> attributs -> schedparam, sp, sizeof(*sp));

  return 0;
}

