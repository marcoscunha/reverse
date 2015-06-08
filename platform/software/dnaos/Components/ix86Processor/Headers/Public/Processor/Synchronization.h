#ifndef PROCESSOR_SYNCHRONIZATION_H
#define PROCESSOR_SYNCHRONIZATION_H

/* static inline long int cpu_compare_and_swap (volatile long int * p_val, long int oldval, long int newval) { */
static inline int32_t cpu_compare_and_swap (volatile int32_t * p_val, int32_t oldval, int32_t newval)
{
    //return 0 => ok
    unsigned char prev;
    
	__asm__ volatile (
        "lock cmpxchgl %1, %2\n"
    	"setne %%al"
		     : "=a" (prev)
		     : "r" (newval), "m" (* p_val), "0" (oldval)
		     : "memory");

	return prev;
}

/* static inline long int cpu_test_and_set (volatile long int * spinlock) { */
static inline int cpu_test_and_set (volatile long int * spinlock)
{
    return cpu_compare_and_swap (spinlock, 0, 1);
}

#endif

