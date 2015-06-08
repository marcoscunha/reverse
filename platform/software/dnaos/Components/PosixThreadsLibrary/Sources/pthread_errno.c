#include <Private/PosixThreads.h>

static int32_t p_errno;

int32_t * __libthread_errno (void)
{
#if 0
  int32_t self_id;
  pthread_t pself;

 self_id = thread_find (NULL);
 thread_get_cookie (self_id, (void **) & pself);

  return & pself -> errorno;
#endif

  return & p_errno;
}
