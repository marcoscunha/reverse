#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
  ASSERT_RETURN (! attr || ! policy, EINVAL);

  *policy = attr -> schedpolicy;
  return 0;
}
