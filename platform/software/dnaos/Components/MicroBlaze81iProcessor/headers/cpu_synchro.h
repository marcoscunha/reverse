#ifndef CPU_SYNCHRO_H
#define CPU_SYNCHRO_H

static inline long int CPU_TEST_AND_SET (volatile long int * spinlock) {
	if (*spinlock == 0) {
		*spinlock = 1;
		return 0;
	}
	else return *spinlock;
}

static inline long int CPU_COMPARE_AND_SWAP (volatile long int * p_val, long int oldval, long int newval) {
	long int val = *p_val;

	if (val == oldval) *p_val = newval;
	return val;
}

#endif

