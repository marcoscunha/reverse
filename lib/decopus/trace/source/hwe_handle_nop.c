
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "hwe_handle.h"

#define POOL_prefix hwe_pool
#define POOL_elem_t hwe_cont
#define POOL_grain  100
#include "pool.h"

#define HWE_HANDLE_NOP_STATS

static hwe_pool_t  hwe_pool;

#ifdef HWE_HANDLE_NOP_STATS
static unsigned cnt, max;
#endif

void handle_start(const char *tracename, int nopt, char * const opt[])
{
	hwe_pool_init(&hwe_pool);
#ifdef HWE_HANDLE_NOP_STATS
	cnt = 0;
	max = 0;
#endif
}

hwe_cont * handle_alloc()
{
#ifdef HWE_HANDLE_NOP_STATS
	cnt++;
	if (cnt > max)
		max = cnt;
#endif
	hwe_cont *res = hwe_pool_get(&hwe_pool);
	res->common.self = (hwe_ref_t) res;
	res->common.refnext = NULL;
	res->common.reflast = NULL;
	return res;
}

void handle_event(hwe_cont *hwe)
{
	do {
#ifdef HWE_HANDLE_NOP_STATS
		cnt--;
#endif
		hwe_cont *next = (hwe_cont *) hwe->common.refnext;
		hwe_pool_put(&hwe_pool, hwe);
		hwe = next;
	} while (hwe != NULL);
}

void handle_stop()
{
	hwe_pool_free(&hwe_pool);
#ifdef HWE_HANDLE_NOP_STATS
	fprintf(stderr, "used %u containers\n", max);
#endif
}

