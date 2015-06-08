#include <Private/PosixThreads.h>
#include <Private/Macros.h>

int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **addr)
{
  ASSERT_RETURN (! attr || ! addr,EINVAL);

  *addr = attr -> stackaddr;
  return 0;
}
