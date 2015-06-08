#ifndef _PTHREAD_H
#define _PTHREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

#include <Core/Core.h>

/*************************
 * Types and definitions *
 *************************/

#define PTHREAD_CANCELED -128
#define PTHREAD_EXITED  (pthread_t)-1
#define PTHREAD_COND_INITIALIZER { NULL }
#define PTHREAD_MUTEX_INITIALIZER { -1 }

typedef uint32_t pthread_state_t;
typedef void *(*pthread_func_t)(void *);

struct pthread_spinlock{
    int32_t lock __attribute__((aligned(32)));
    int32_t spare[7];
};

#ifdef WRITEBACK
typedef volatile struct pthread_spinlock pthread_spinlock_t;
#else
typedef volatile unsigned int pthread_spinlock_t;
#endif

typedef struct pthread_bootstrap
{
  thread_handler_t handler;
  void * argument;
}
* pthread_bootstrap_t;

enum { PTHREAD_CREATE_JOINABLE, PTHREAD_CREATE_DETACHED };
enum { PTHREAD_INHERIT_SCHED, PTHREAD_EXPLICIT_SCHED };
enum { SCHED_OTHER, SCHED_FIFO, SCHED_RR, SCHED_PSET };
enum { PTHREAD_SCOPE_SYSTEM, PTHREAD_SCOPE_THREAD };
enum { PTHREAD_MUTEX_FAST_NP };
enum { PTHREAD_THREAD_SHARED };
enum { PTHREAD_BARRIER_SERIAL_THREAD };
enum { PTHREAD_CANCEL_DEFERRED, PTHREAD_CANCEL_ASYNCHRONOUS };
enum { PTHREAD_CANCEL_DISABLE, PTHREAD_CANCEL_ENABLE };
enum { LONGJMP_ON, LONGJMP_OFF };
enum { REALTIME_OFF, REALTIME_ON };
enum { RT_RUN, RT_RESET };
enum { RT_MODE_1, RT_MODE_2 };
enum { PTHREAD_NOPROCID = 0xFFFFFFFF };

typedef struct sched_param
{
   int priority;
   int deadline;
   int procs;
}
sched_param_t;

/*********
 * MUTEX *
 *********/

typedef struct pthread_mutex
{
  int32_t lock __attribute__((aligned(32)));
  int32_t spare[7];
  int32_t flags;
  int32_t status;
}
pthread_mutex_t;

typedef struct pthread_mutexattr
{

}
pthread_mutexattr_t;

enum
{
  MUTEX_UNLOCKED,
  MUTEX_LOCKED
};

/**************
 * CONDITIONS *
 **************/

typedef struct pthread_cond
{
  int32_t semaphore __attribute__((aligned(32)));
  int32_t spare[7];
  int32_t count;
}
pthread_cond_t;

typedef struct pthread_condattr
{
}
pthread_condattr_t;
 
/***********
 * BARRIER *
 ***********/

typedef volatile struct pthread_barrier
{
  uint32_t threshold;
  uint32_t count;
  uint32_t sem;
}
pthread_barrier_t;

typedef struct pthread_barrierattr
{
}
pthread_barrierattr_t;

/***********
 * PTHREAD *
 ***********/

typedef struct pthread_attr
{
  void *stackaddr;
  size_t stacksize;
  uint32_t detachstate;
  uint32_t schedinherited;
  uint32_t schedpolicy;
  uint32_t procid;
  sched_param_t schedparam;
  char * name;
}
pthread_attr_t;

typedef struct pthread
{
  struct pthread  *next;
  struct pthread  *prev;

  pthread_attr_t *attributs;                /* Attributs of the thread */

  int32_t tid;

  struct pthread *father;                   /* Forking thread */
  uint32_t children;                 /* Number of forked threads */

  uint8_t cancel_type;            /* type */
  uint8_t cancel_state;           /* state */
  bool cancel_bool;            /* boolean */
  uint8_t cancel_async;           /* delete asap */

  int32_t errorno;
}
* pthread_t;

/***********
 * METHODS *
 ***********/

extern int32_t pthread_bootstrap (void *);

extern int pthread_create (pthread_t *,pthread_attr_t *,pthread_func_t, void *);
extern void pthread_exit (void *);
extern int pthread_join (pthread_t, void **);
extern int pthread_spin_destroy (pthread_spinlock_t *);
extern int pthread_spin_init (pthread_spinlock_t *, int);
extern int pthread_lock_acquire (pthread_spinlock_t *);
extern int pthread_lock_release (pthread_spinlock_t *);
extern int pthread_mutex_init (pthread_mutex_t *, const pthread_mutexattr_t *);
extern int pthread_mutex_destroy (pthread_mutex_t *);
extern pthread_t pthread_self (void);
extern int pthread_equal (pthread_t, pthread_t);
extern int pthread_mutex_lock (pthread_mutex_t *); 
extern int pthread_mutex_trylock (pthread_mutex_t *); 
extern int pthread_mutex_unlock (pthread_mutex_t *); 
extern int pthread_cond_broadcast (pthread_cond_t *);
extern int pthread_cond_signal (pthread_cond_t *);
extern int pthread_cond_wait (pthread_cond_t *, pthread_mutex_t *);
extern int pthread_cond_timedwait (pthread_cond_t *, pthread_mutex_t *, const struct timespec *);
extern int pthread_cond_init (pthread_cond_t *, pthread_condattr_t *);
extern int pthread_cond_destroy (pthread_cond_t *);
extern int pthread_setcancelstate (int, int *);
extern int pthread_setcanceltype (int, int *);
extern int pthread_cancel (pthread_t);
extern void pthread_testcancel (void);

extern int pthread_attr_init (pthread_attr_t *);
extern int pthread_attr_destroy (pthread_attr_t *);
extern int pthread_attr_setschedpolicy (pthread_attr_t *, int);
extern int pthread_attr_getschedpolicy (const pthread_attr_t *, int *);
extern int pthread_attr_setschedparam (pthread_attr_t *, const struct sched_param *);
extern int pthread_attr_getschedparam (const pthread_attr_t *, struct sched_param *);
extern int pthread_attr_setinheritsched (pthread_attr_t *, int);
extern int pthread_attr_getinheritsched (const pthread_attr_t *, int *);
extern int pthread_attr_setstackaddr (pthread_attr_t *, void *);
extern int pthread_attr_getstackaddr (const pthread_attr_t *, void **);
extern int pthread_attr_setstacksize (pthread_attr_t *, size_t);
extern int pthread_attr_getstacksize (const pthread_attr_t *, size_t *);
extern int pthread_attr_setdetachstate (pthread_attr_t *, int);
extern int pthread_attr_getdetachstate (const pthread_attr_t *, int *);

extern int pthread_setschedparam (pthread_t, int, const struct sched_param *);
extern int pthread_getschedparam (pthread_t, int *, struct sched_param *);
#endif
