#include <Private/PosixThreads.h>

pthread_t pthread_self(void)
{
#if 0
  int32_t self_id; 
  pthread_t pself;
  
  self_id = thread_find (NULL);
  thread_get_cookie (self_id, (void **) & pself);

  return pself;
#endif

  return NULL;
}
