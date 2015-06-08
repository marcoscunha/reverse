#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_setcanceltype(int type, int *oldtype)
{
#if 0
  pthread_t self;

  ASSERT_RETURN(type != PTHREAD_CANCEL_ASYNCHRONOUS && type != PTHREAD_CANCEL_DEFERRED,EINVAL);

  self = (pthread_t) core_thread_self() -> private_data;
  if (oldtype != NULL) *oldtype = self -> cancel_type;
  self -> cancel_type = type;
#endif

  return 0;
}
