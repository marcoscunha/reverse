#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
  ASSERT_RETURN(!attr,EINVAL);

  attr->schedpolicy = policy;
  return 0;
}
