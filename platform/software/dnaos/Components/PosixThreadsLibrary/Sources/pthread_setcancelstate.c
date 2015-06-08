#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_setcancelstate(int state, int *oldstate)
{
#if 0
  pthread_t self;
 
  ASSERT_RETURN(state != PTHREAD_CANCEL_DISABLE && state != PTHREAD_CANCEL_ENABLE,EINVAL);

  self = (pthread_t) core_thread_self() -> private_data;
  if (oldstate != NULL) *oldstate = self -> cancel_state;
  self -> cancel_state = state;
#endif

  return 0;
}
