#ifndef PROCESSOR_SYNCHRONIZATION_H
#define PROCESSOR_SYNCHRONIZATION_H

static inline long int cpu_test_and_set (volatile long int * spinlock)
{
  register long int ret, temp = 1;

  __asm__ volatile
    ("/* Inline spinlock test & set */\n\t"
     "1:\n\t"
     "ll  %0,%3\n\t"
     ".set push\n\t"
     ".set  noreorder\n\t"
     "bnez %0,2f\n\t"
     "li %1,1\n\t"
     ".set pop\n\t"
     "sc  %1,%2\n\t"
     "beqz %1,1b\n"
     "2:\n\t"
     "/* End spinlock test & set */"
     : "=&r" (ret), "=&r" (temp), "=m" (*spinlock)
     : "m" (*spinlock)
     : "memory");

  return ret;
}

static inline long int cpu_compare_and_swap (volatile long int * p_val,
    long int oldval, long int newval)
{
  register long int ret;

  __asm__ volatile
    ("/* Inline compare & swap */\n\t"
     "__start_cas:\n\t"
     "ll  %0,%4\n\t"
     ".set push\n"
     ".set  noreorder\n\t"
     "bne  %0,%2, __end_cas\n\t"
     "move %0,%3\n\t"
     ".set pop\n\t"
     "sc  %0,%1\n\t"
     "xori %0, %0, 1\r\n"
     "__end_cas:\n\t"
     "/* End compare & swap */"
     : "=&r" (ret), "=m" (*p_val)
     : "r" (oldval), "r" (newval), "m" (*p_val)
     : "memory");

  return ret;
}

#endif

