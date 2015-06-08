#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *h)
{
  ASSERT_RETURN(!attr || !h,EINVAL);

  *h = attr -> schedinherited;
  return 0;
}
