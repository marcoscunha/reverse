
#ifndef __HWETRACE_STAT_INTERNAL_H__
#define __HWETRACE_STAT_INTERNAL_H__

#ifdef HWETRACE_STAT
static struct hwetrace_stat evstat = {};
#endif

const struct hwetrace_stat * hwetrace_stat_get()
{
#ifdef HWETRACE_STAT
	return &evstat;//see below
#else
	return NULL;
#endif	
}

#ifndef HWETRACE_STAT

#define HWETRACE_STAT_PRINT(fmt, ...)

#define HWETRACE_TRACESTAT_FIELD(s)
#define HWETRACE_TRACESTAT_INIT(s)
#define HWETRACE_TRACESTAT_IN(s1,s2,b)
#define HWETRACE_TRACESTAT_OUT(s1,s2,c)
#define HWETRACE_TRACESTAT_PRINT(str,s)

#define HWETRACE_EVENTSTAT_LOG(x, y)
#define HWETRACE_EVENTSTAT_PRINT()

#else

#include "events/hwe_events.h"

#define HWETRACE_STAT_PRINT(fmt, ...) HWETRACE_PRINT(fmt, ## __VA_ARGS__)

struct hwe_tracestat {
	unsigned ev_cur; //nb of currently owned event
	unsigned co_cur; //nb of currently owned containers 
	unsigned cpe_max; //nb max of container per event
	uint64_t co_tot; //total nb of containers commited
	uint64_t ev_tot; //total nb of events commited
};

#define HWETRACE_TRACESTAT_FIELD(x) struct hwe_tracestat x

static void HWETRACE_TRACESTAT_INIT(struct hwe_tracestat *s)
{
	s->ev_cur = 0;
	s->co_cur = 0;
	s->cpe_max = 0;
	s->co_tot = 0;
	s->ev_tot = 0;
}

static void HWETRACE_TRACESTAT_PRINT(const char *str, const struct hwe_tracestat *s)
{
	HWETRACE_PRINT("%s stats:"
			" %.2f Mev(tot) / %.2f Mco(tot) / %u co/ev(max) /"
			" %u ev(cur) / %u co(cur) \n",
			str,	
			(1.0 * s->ev_tot) / 1000000.0,
			(1.0 * s->co_tot) / 1000000.0,
			       s->cpe_max,
					 s->ev_cur,
			       s->co_cur);
}

static void HWETRACE_TRACESTAT_OUT(struct hwe_tracestat *s1, struct hwe_tracestat *s2, bool ev)
{
	s1->co_cur += 1;
	s2->co_cur += 1;
	if (ev) {
		s1->ev_cur += 1;
		s2->ev_cur += 1;
	}
}

static void HWETRACE_TRACESTAT_IN(struct hwe_tracestat *s1, struct hwe_tracestat *s2, hwe_head_cont *cont)
{
	unsigned cpe = 0;
	do {
		cpe += 1;
		cont = cont->refnext;	
	} while (cont);
	
	s1->co_tot += cpe;
	s2->co_tot += cpe;
	
	s1->ev_tot += 1;
	s2->ev_tot += 1;
	
	s1->ev_cur -= 1;
	s1->co_cur -= cpe;
	if (cpe > s1->cpe_max)
		s1->cpe_max = cpe;

	s2->ev_cur -= 1;
	s2->co_cur -= cpe;
	if (cpe > s2->cpe_max)
		s2->cpe_max = cpe;
}

void HWETRACE_EVENTSTAT_LOG(hwe_device_t device, hwe_cont *cont)
{
	/*
	 * global counts
	 */
	evstat.global.event++;
	{
		hwe_head_cont *head = &cont->common;
		do {
			evstat.global.container++;
			head = head->refnext;
		} while (head);
	}
	switch(cont->common.head.type) {
		case HWE_INFO:
			evstat.global.info++;
			return;
			break;
		case HWE_NULL:
			evstat.global.null++;
			break;
		default:
			break;
	}

	switch(device) {
			/*
			 * cpu counts
			 */
		case HWE_PROCESSOR:
			switch(cont->common.head.type) {
				case HWE_INST32:
					evstat.cpu.inst++;
					return;
				case HWE_EXCEP32:
					evstat.cpu.excep++;
					return;
				case HWE_NULL:
					evstat.cpu.cancel++;
					return;
				case HWE_MEMGL:
					switch (cont->mem.body.global.access) {
						case HWE_MEM_SYNC:
							evstat.cpu.sync++;
							return;
						default:
							break;
					}
					break;
				case HWE_MEM32:
					if (cont->mem.body.mem32.inst) {
						switch (cont->mem.body.mem32.access) {
							case HWE_MEM_LOAD:
								evstat.cpu.ifetch++;
								return;
							default:
								break;
						}
					} else {
						switch (cont->mem.body.mem32.access) {
							case HWE_MEM_LOAD:
								evstat.cpu.ld++;
								return;
							case HWE_MEM_STORE:
								evstat.cpu.st++;
								return;
							case HWE_MEM_LL:
								evstat.cpu.ll++;
								return;
							case HWE_MEM_SC:
								if (cont->mem.body.mem32.condfailed)
									evstat.cpu.sc_ko++;
								else
									evstat.cpu.sc++;
								return;
							case HWE_MEM_INVAL:
								evstat.cpu.inval++;
								return;
                            // TODO: Include new invalidations and replaces
							default:
								break;
						}
					}
					break;
				default:
					break;
			}
			break;

			/*
			 * mem count
			 */
		case HWE_MEMORY:
			switch(cont->common.head.type) {
				case HWE_MEMACK:
					evstat.memory.ack++;
					return;
				case HWE_MEM32:
					evstat.memory.req++;
					return;
				default:
					break;
			}
			break;

			/*
			 * cache
			 */
		case HWE_CACHE:
			switch(cont->common.head.type) {
				case HWE_MEM32:
					switch (cont->mem.body.mem32.access) {
						case HWE_MEM_LOAD:
							evstat.cache.fill++;
							return;
						case HWE_MEM_INVAL:
							evstat.cache.inval++;
							return;
                        // TODO: Include new invalidations and replaces
						default:
							break;
					}
					break;
				case HWE_SPREAD:
					switch(cont->common.head.expected) {
						case 1:
							evstat.cache.spread1++;
							return;
						case 2:
							evstat.cache.spread2++;
							return;
						default:
							break;
					}
				case HWE_MEMACK:
					evstat.cache.ack++;
					return;
				default:
					break;
			}
			break;
			
			/*
			 * buffer
			 */
		case HWE_WRITEBUFFER:
			switch(cont->common.head.type) {
				case HWE_MEM32:
					if (cont->common.head.expected > 1)
						evstat.buffer.req_update++;
					else
						evstat.buffer.request++;
					return;
				case HWE_SPREAD:
					evstat.buffer.spread++;
					return;
				case HWE_MEMACK:
					if (cont->common.head.expected) {
						evstat.buffer.write_ack++;
					} else {
						evstat.buffer.read_ack++;
					}
					return;
				default:
					break;
			}
			break;
        case HWE_PERIPHERAL:
        break;
	}
	
	evstat.global.missed++;
}

#define HWETRACE_EVENTSTAT_PRINT_FIELD(grp,fld) \
	HWETRACE_PRINT("Event stats %6s.%10s = %'"PRIu64"\n", #grp, #fld, evstat . grp . fld)

void HWETRACE_EVENTSTAT_PRINT()
{
	HWETRACE_EVENTSTAT_PRINT_FIELD(global,event);
	HWETRACE_EVENTSTAT_PRINT_FIELD(global,container);
	HWETRACE_EVENTSTAT_PRINT_FIELD(global,info);
	HWETRACE_EVENTSTAT_PRINT_FIELD(global,null);
	HWETRACE_EVENTSTAT_PRINT_FIELD(global,missed);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,inst);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,excep);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,ifetch);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,ld);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,st);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,ll);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,sc);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,sc_ko);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,inval);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,sync);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cpu,cancel);
	HWETRACE_EVENTSTAT_PRINT_FIELD(buffer,write_ack);
	HWETRACE_EVENTSTAT_PRINT_FIELD(buffer,request);
	HWETRACE_EVENTSTAT_PRINT_FIELD(buffer,req_update);
	HWETRACE_EVENTSTAT_PRINT_FIELD(buffer,spread);
	HWETRACE_EVENTSTAT_PRINT_FIELD(buffer,read_ack);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cache,ack);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cache,fill);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cache,inval);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cache,spread1);
	HWETRACE_EVENTSTAT_PRINT_FIELD(cache,spread2);
	HWETRACE_EVENTSTAT_PRINT_FIELD(memory,ack);
	HWETRACE_EVENTSTAT_PRINT_FIELD(memory,req);
}

#endif

#endif

