#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_attr_setinheritsched(pthread_attr_t *attr, int h)
{
  ASSERT_RETURN(!attr,EINVAL);

  attr->schedinherited = h;
  return 0;
}
