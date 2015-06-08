#include <string.h>
#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_attr_getschedparam(const pthread_attr_t *attr, sched_param_t *param)
{
  ASSERT_RETURN (! attr || ! param, EINVAL);

  memcpy(param, & attr -> schedparam, sizeof (*param));
  return 0;
}
