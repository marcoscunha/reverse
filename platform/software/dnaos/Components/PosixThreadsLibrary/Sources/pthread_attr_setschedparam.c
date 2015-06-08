#include <string.h>
#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_attr_setschedparam(pthread_attr_t *attr, const sched_param_t *param)
{
  ASSERT_RETURN(!attr || !param,EINVAL);

  memcpy(&attr->schedparam, param, sizeof(*param));
  return 0;
}
