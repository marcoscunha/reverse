#ifndef PROCESSOR_SYNCHRONIZATION_H
#define PROCESSOR_SYNCHRONIZATION_H

static inline long int cpu_test_and_set (volatile long int * spinlock)
{
  long int ret = *spinlock;

  *spinlock = 1;
  return ret;
}

static inline long int cpu_compare_and_swap (volatile long int * p_val,
    long int oldval, long int newval)
{
  if (*p_val == oldval)
  {
    *p_val = newval;
    return 1;
  }
  else
  {
    return 0;
  }
}

#endif

