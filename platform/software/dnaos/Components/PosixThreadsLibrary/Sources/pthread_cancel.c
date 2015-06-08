#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_cancel(pthread_t thread)
{
  ASSERT_RETURN (thread == NULL, ESRCH);
  ASSERT_RETURN (thread -> cancel_state != PTHREAD_CANCEL_DISABLE
      && thread -> cancel_state != PTHREAD_CANCEL_ENABLE, EINVAL);

  ASSERT_RETURN (thread -> cancel_type != PTHREAD_CANCEL_DEFERRED
      && thread -> cancel_type != PTHREAD_CANCEL_ASYNCHRONOUS, EINVAL);

  if (thread -> cancel_state == PTHREAD_CANCEL_DISABLE) return 0;
  if (thread -> cancel_type == PTHREAD_CANCEL_DEFERRED) {
    thread -> cancel_bool = true;
    return 0;
  }

  // FIXME : Not implemented. We need to decide
  // wether if cancel means exit or smtgh else
  return 0;
}
