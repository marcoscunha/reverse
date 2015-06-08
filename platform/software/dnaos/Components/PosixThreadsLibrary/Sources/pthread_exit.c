#include <Private/PosixThreads.h>

void pthread_exit(void *retval)
{

  /* Get the thread waiting for the termination of the current thread
   * and make it runnable */

  // FIXME : Not yet implemented
}
