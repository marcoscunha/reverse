#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *size)
{
  ASSERT_RETURN(!attr || !size,EINVAL);

  *size = attr->stacksize;
  return 0;
}
