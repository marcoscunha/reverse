#include <Private/PosixThreads.h>

int pthread_cond_init(pthread_cond_t *condition, pthread_condattr_t *condattr)
{
  static int32_t index = 0;
  char name[32];
  status_t status = DNA_OK;

  sprintf (name, "pthread_cond_%ld", index ++);
  condition -> count = 0;
  status = semaphore_create (name, 0, & condition -> semaphore);

  if (status != DNA_OK)
  {
    switch (status)
    {
      case DNA_NO_MORE_SEM :
        return EAGAIN;
      case DNA_OUT_OF_MEM :
        return ENOMEM;
      default :
        return EINVAL;
    }
  }

  return 0;
}
