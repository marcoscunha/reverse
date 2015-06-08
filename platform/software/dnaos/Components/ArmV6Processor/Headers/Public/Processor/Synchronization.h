#ifndef PROCESSOR_SYNCHRONIZATION_H
#define PROCESSOR_SYNCHRONIZATION_H

long int cpu_test_and_set (volatile long int * spinlock);
long int cpu_compare_and_swap (volatile long int * p_val,
    long int oldval, long int newval);


/*static inline long int cpu_test_and_set (volatile long int * spinlock)
{
  register long int ret, temp = 1; 

  __asm__ volatile
    ("\n"
     "__start_tst:\n"
     "mcr p15, 0, %4, c7, c6, 1\n" // cpu_dcache_inv_addr
     "ldrex   %0, %3\n"
     "cmp     %0, #0\n"
     "strexeq %0, %2, %1\n"
     : "=&r" (ret), "=m" (*spinlock)
     : "r" (temp), "m" (*spinlock)
     : "memory");

  return ret;
}

static inline long int cpu_compare_and_swap (volatile long int * p_val,
    long int oldval, long int newval)
{
  register long int ret;

  __asm__ volatile
    ("\n"
     "__start_cas:\n"
     "ldrex   %0, [%3]\n"
     "cmp     %0, %1\n"
     "strexeq %0, %2, [%3]\n"
     : "=&r" (ret)
     : "r" (oldval), "r" (newval), "r" (p_val)
     : "memory");

  return ret;
}
*/
#endif

