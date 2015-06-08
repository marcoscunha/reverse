#include <stdio.h>
#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_join(pthread_t thread, void **value_ptr)
{
  unsigned int status = 0;

  ASSERT_RETURN (thread == NULL, EINVAL);

  thread_wait (thread -> tid, (int32_t *) value_ptr);
  return status;
}
