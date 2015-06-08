#include <Private/PosixThreads.h>

int pthread_mutex_init (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
  static int32_t index = 0;
  char name[32];
  status_t status = DNA_OK;

  if (mutex == NULL) return EFAULT;

  sprintf (name, "pthread_mutex_%ld", index ++);
  status = semaphore_create (name, 1, & mutex -> lock);

  if(status == DNA_NO_MORE_SEM) dna_printf("No more semaphores\n");

  if (status != DNA_OK) return EINVAL;
  return 0;
}
