
#ifndef __HWETRACE_STAT_H__
#define __HWETRACE_STAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct hwetrace_stat {
#define _HWE_global 0
	struct {
		uint64_t event;
		uint64_t container;
		uint64_t info;
		uint64_t null;
		uint64_t missed;
	} global;
#define _HWE_cpu _HWE_global + 5
	struct {
		uint64_t inst;
		uint64_t excep;
		uint64_t ifetch;
		uint64_t ld;
		uint64_t st;
		uint64_t ll;
		uint64_t sc;
		uint64_t sc_ko;
		uint64_t sync;
		uint64_t inval;
		uint64_t cancel;
	} cpu;
#define _HWE_buffer _HWE_cpu + 11
	struct {
		uint64_t read_ack;
		uint64_t write_ack;
		uint64_t request;
		uint64_t spread;
		uint64_t req_update;
	} buffer;
#define _HWE_cache _HWE_buffer + 5
	struct {
		uint64_t ack;
		uint64_t fill;
		uint64_t inval;
		uint64_t spread1;
		uint64_t spread2;
	} cache;
#define _HWE_memory _HWE_cache + 5
	struct {
		uint64_t ack;
		uint64_t req;
	} memory;
#define _HWE_TOTAL _HWE_memory + 2
};

extern uint64_t hwetrace_event_count();

extern const struct hwetrace_stat * hwetrace_stat_get();

__attribute__((__unused__))
static const unsigned hwetrace_stat_nb_fields = _HWE_TOTAL;

__attribute__((__unused__))
static uint64_t hwetrace_stat_get_field(
		const struct hwetrace_stat *stat, 
		unsigned field, 
		const char **group, 
		const char **name)
{
	const char *ptr;
	if (group == NULL)
		group = &ptr;
	if (name == NULL)
		name = &ptr,
	*group = NULL;
	*name  = NULL;
	uint64_t res = 0;
	switch(field) {
#define _CASE(g,i,n) case _HWE_ ## g + i :{\
	*group = #g ;\
	*name = #n ;\
	res = stat-> g . n ;\
} break;
			_CASE(global, 0, event);
			_CASE(global, 1, container);
			_CASE(global, 2, info);
			_CASE(global, 3, null);
			_CASE(global, 4, missed);
			
			_CASE(cpu, 0, inst);
			_CASE(cpu, 1, excep);
			_CASE(cpu, 2, ifetch);
			_CASE(cpu, 3, ld);
			_CASE(cpu, 4, st);
			_CASE(cpu, 5, ll);
			_CASE(cpu, 6, sc);
			_CASE(cpu, 7, sc_ko);
			_CASE(cpu, 8, sync);
			_CASE(cpu, 9, inval);
			_CASE(cpu, 10, cancel);
			
			_CASE(buffer, 0, read_ack);
			_CASE(buffer, 1, write_ack);
			_CASE(buffer, 2, request);
			_CASE(buffer, 3, spread);
			_CASE(buffer, 4, req_update);

			_CASE(cache, 0, ack);
			_CASE(cache, 1, fill);
			_CASE(cache, 2, inval);
			_CASE(cache, 3, spread1);
			_CASE(cache, 4, spread2);

			_CASE(memory, 0, ack);
			_CASE(memory, 1, req);
#undef _CASE
	}
	return res;
}

#undef _HWE_global
#undef _HWE_cpu
#undef _HWE_buffer
#undef _HWE_cache
#undef _HWE_memory
#undef _HWE_TOTAL

#ifdef __cplusplus
}
#endif

#endif

