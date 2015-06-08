#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t size)
{
  ASSERT_RETURN(!attr || size == 0,EINVAL);
  ASSERT_RETURN(size%4,EINVAL);
  attr->stacksize = size;
  return 0;
}
