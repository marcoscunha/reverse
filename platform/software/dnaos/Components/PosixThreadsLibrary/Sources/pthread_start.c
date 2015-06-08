#include <malloc.h>
#include <Private/PosixThreads.h>
#include <Private/Macros.h>

extern int main (void);

int __libthread_start (void)
{
  int32_t my_id;
  pthread_t wrapper = malloc (sizeof (struct pthread));

  thread_find (NULL, & my_id);

  wrapper -> attributs = malloc (sizeof (pthread_attr_t));
  wrapper -> father = NULL;
  wrapper -> children = 0;
  wrapper -> cancel_type = PTHREAD_CANCEL_ASYNCHRONOUS;
  wrapper -> cancel_state = PTHREAD_CANCEL_ENABLE;
  wrapper -> cancel_bool = false;
  wrapper -> tid = my_id;
  wrapper -> attributs -> stacksize = 0;
  wrapper -> attributs -> stackaddr = NULL;
  wrapper -> attributs -> detachstate = PTHREAD_CREATE_JOINABLE;
  wrapper -> attributs -> schedinherited = PTHREAD_EXPLICIT_SCHED;
  wrapper -> attributs -> schedpolicy = SCHED_FIFO;
  wrapper -> attributs -> procid = PTHREAD_NOPROCID;

  return main ();
}
