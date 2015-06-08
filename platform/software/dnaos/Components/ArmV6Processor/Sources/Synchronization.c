#include<Processor/Cache.h>
long int cpu_test_and_set (volatile long int * spinlock)
{
  register long int ret, temp = 1; 
  __asm__ volatile
    ("\n"
     "__start_tst:\n"
#ifdef WRITEBACK
     "mcr p15, 0, %4, c7, c14, 1\n"
#elif defined WRITETHROUGH
     "mcr p15, 0, %4, c7, c6, 1\n"
#endif
     "ldrex   %0, %3\n"
     "cmp     %0, #0\n"
     "strexeq %0, %2, %1\n"
     : "=&r" (ret), "=m" (*spinlock)
     : "r" (temp), "m" (*spinlock), "r"(spinlock)
     : "memory");
  return ret;
}

long int cpu_compare_and_swap (volatile long int * p_val,
    long int oldval, long int newval)
{
  long int ret;

  __asm__ volatile
    ("\n"
     "__start_cas:\n"
#ifdef WRITEBACK
     "mcr p15, 0, %3, c7, c14, 1\n"
#elif defined WRITETHROUGH
     "mcr p15, 0, %3, c7, c6, 1\n"
#endif
     "mcr p15, 0, %3, c7, c6, 1\n"
     "ldrex   %0, [%3]\n"
     "cmp     %0, %1\n"
     "strexeq %0, %2, [%3]\n"
     : "=&r" (ret)
     : "r" (oldval), "r" (newval), "r" (p_val)
     : "memory");

  return ret;
}

