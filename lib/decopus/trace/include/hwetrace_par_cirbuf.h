#ifndef _HWETRACE_PAR_CIRBUF_H_
#define _HWETRACE_PAR_CIRBUF_H_

#include <time.h>
#include <pthread.h>
#include <errno.h>

//cache line size alignement (in bytes)
#ifndef HWEPAR_ALIGN
//for x86
#define HWEPAR_ALIGN 64
#endif

#ifndef HWEPAR_BUFSIZE
#error "HWEPAR_BUFSIZE undefined"
#endif

#ifndef HWEPAR_NBBUF
#error "HWEPAR_NBBUF undefined"
#endif

#ifndef HWEPAR_BUFSTART
#define HWEPAR_BUFSTART 1
#endif
#if HWEPAR_BUFSTART >= HWEPAR_BUFSIZE || HWEPAR_BUFSTART <= 0
#error "BUFSTART should be 0 < ... < HWEPAR_BUFSIZE"
#endif

#ifndef HWEPAR_MODE
#define HWEPAR_MODE 1
#endif
#if HWEPAR_MODE < 0 || HWEPAR_MODE > 3
#error "Invalid parallel mode HWEPAR_MODE"
#endif

#define TRACER_PRINT(fmt, ...)  HWETRACE_PRINT("[TRACER] " fmt, ## __VA_ARGS__)
#define HANDLER_PRINT(fmt, ...) HWETRACE_PRINT("[HANDLER] " fmt, ## __VA_ARGS__)

#ifdef HWEPAR_DEBUG
#define DEBUG_ASSERT(cond) assert(cond)
#define DEBUG_DO(code) code
#else
#define DEBUG_ASSERT(cond)
#define DEBUG_DO(code)
#endif

#ifdef HWEPAR_VERB_TRACER
#define TRACER_VPRINT(fmt, ...) TRACER_PRINT(fmt, ## __VA_ARGS__)
#else
#define TRACER_VPRINT(...)
#endif

#ifdef HWEPAR_VERB_HANDLER
#define HANDLER_VPRINT(fmt, ...) HANDLER_PRINT(fmt, ## __VA_ARGS__)
#else
#define HANDLER_VPRINT(...)
#endif

struct hwepar {
	//buffers space
	void *alloc;
	union hwepar_buffer_align *volatile buffers;
	//the handler's thread
	pthread_t handler;

	//tracer local indexes
	unsigned bufrdy;
	unsigned bufidx;
	volatile struct hwepar_buffer *buf;

#if HWEPAR_MODE <= 1
	hwe_cont *volatile *array;
	int full_idx;
	int empty_idx;
#else
	hwe_cont *volatile *full_ptr;
	hwe_cont *volatile *empty_ptr;
	hwe_cont *volatile *end_ptr;
#endif

	//shared index for the circular buffers
	//the handler's thread produce clean buffers
	//and the tracer's thread we use them
	char pad0[HWEPAR_ALIGN];
	volatile unsigned handler_idx;
	pthread_mutex_t handler_mutex;
	pthread_cond_t handler_cond;
	char pad1[HWEPAR_ALIGN];
	volatile unsigned tracer_idx;
	pthread_cond_t tracer_cond;
	pthread_mutex_t tracer_mutex;
	pthread_condattr_t tracer_condattr;
	char pad2[HWEPAR_ALIGN];
};

#ifdef HWETRACE_PARSTAT
static uint64_t tracer_wait;
static uint64_t tracer_sign;
static uint64_t tracer_sync;
static uint64_t tracer_buff;
#endif

//a buffer
struct hwepar_buffer {
	//full and empty pointer into the array
#if HWEPAR_MODE <= 1
	int empty_idx;
	int full_idx;
#else
	hwe_cont **empty;
	hwe_cont **full;
#endif
	//data array we have: full_idx <= empty_idx <= BUFSIZE
	// [0] [1] .... [full_idx-1]           ptrs for full events
	// [full_idx] ... [empty_idx-1]        some invalid ptrs
	// [empty_idx] ... [BUFSIZE-1]  ptrs for empty events
	hwe_cont *array[HWEPAR_BUFSIZE];
};
#if HWEPAR_MODE == 0
#define FULL_IDX(ptr)  ((ptr)->full_idx)
#define EMPTY_IDX(ptr) ((ptr)->empty_idx)
#elif HWEPAR_MODE == 1
#define FULL_IDX(ptr)  ((ptr)->full_idx + 1)
#define EMPTY_IDX(ptr) ((ptr)->empty_idx + 1)
#elif HWEPAR_MODE == 2
#define FULL_IDX(ptr)  ((ptr)->full - &(ptr)->array[0])
#define EMPTY_IDX(ptr) ((ptr)->empty - &(ptr)->array[0])
#elif HWEPAR_MODE == 3
#define FULL_IDX(ptr)  ((ptr)->full - &(ptr)->array[-1])
#define EMPTY_IDX(ptr) ((ptr)->empty - &(ptr)->array[-1])
#endif
//a buffer with a size aligned on HWEPAR_ALIGN
union hwepar_buffer_align {
	struct hwepar_buffer buffer;
	char padding[(((sizeof(struct hwepar_buffer) - 1) / HWEPAR_ALIGN) + 1) * HWEPAR_ALIGN];
} __attribute__((__packed__));

static void* hwepar_thread(void *);
static void hwepar_switchbuffer(struct hwepar *hwepar);

/*
 * init
 */
static void hwepar_init(struct hwepar *hwepar) {
	//allocate space for buffers
	hwepar->alloc = malloc(sizeof(union hwepar_buffer_align) * HWEPAR_NBBUF + HWEPAR_ALIGN);
	//compute buffers to be aligned on HWEPAR_ALIGN basis
	hwepar->buffers = (void *) ((((uintptr_t) hwepar->alloc) + HWEPAR_ALIGN) - (((uintptr_t) hwepar->alloc) % HWEPAR_ALIGN));

	//init the buffers
	for (int i = 0; i < HWEPAR_NBBUF; i++) {
#if HWEPAR_MODE == 0
		hwepar->buffers[i].buffer.full_idx = 0;
		hwepar->buffers[i].buffer.empty_idx = HWEPAR_BUFSIZE;
#elif HWEPAR_MODE == 1
		hwepar->buffers[i].buffer.full_idx = -1;
		hwepar->buffers[i].buffer.empty_idx = HWEPAR_BUFSIZE - 1;
#elif HWEPAR_MODE == 2
		hwepar->buffers[i].buffer.full = &hwepar->buffers[i].buffer.array[0];
		hwepar->buffers[i].buffer.empty = &hwepar->buffers[i].buffer.array[HWEPAR_BUFSIZE];
#elif HWEPAR_MODE == 3
		hwepar->buffers[i].buffer.full = &hwepar->buffers[i].buffer.array[0] - 1;
		hwepar->buffers[i].buffer.empty = &hwepar->buffers[i].buffer.array[HWEPAR_BUFSIZE] - 1;
#endif
#ifdef HWEPAR_DEBUG
		for (int j = 0; j < HWEPAR_BUFSIZE; j++) {
			hwepar->buffers[i].buffer.array[j] = NULL;
		}
#endif
	}
	//init the indexes so that the handler have HWEPAR_BUFSIZE-1 (the max possible) to clean
	hwepar->tracer_idx = 0;
	hwepar->handler_idx = 1;
	//since tracer_idx == handler_idx, the circular buffer is considered fully busy
	//(ie: no ready buffers for the tracer yet)
	
	//the tracer start with the first buffer (which is empty)
	//so it will starts by try to get a new one
	hwepar->bufrdy = 1;
	hwepar->bufidx = 0;
	hwepar->buf = &hwepar->buffers[0].buffer;
#if HWEPAR_MODE == 0
	hwepar->empty_idx = HWEPAR_BUFSIZE;
	hwepar->full_idx = 0;
	hwepar->array = NULL;
#elif HWEPAR_MODE == 1
	hwepar->empty_idx = HWEPAR_BUFSIZE - 1;
	hwepar->full_idx = -1;
	hwepar->array = NULL;
#elif HWEPAR_MODE >= 2
	hwepar->full_ptr = hwepar->buf->full;
	hwepar->empty_ptr = hwepar->buf->empty;
	hwepar->end_ptr  = hwepar->buf->empty;
#endif
	
	pthread_mutex_init(&hwepar->handler_mutex, NULL);
	pthread_mutex_init(&hwepar->tracer_mutex, NULL);
	pthread_cond_init(&hwepar->handler_cond, NULL);
	pthread_condattr_init(&hwepar->tracer_condattr);
	pthread_condattr_setclock(&hwepar->tracer_condattr, CLOCK_REALTIME);
	pthread_cond_init(&hwepar->tracer_cond, &hwepar->tracer_condattr);

	HWETRACE_PRINT("Parallel mode %d using %d buffers (size %d, start index %d)\n", HWEPAR_MODE, HWEPAR_NBBUF, HWEPAR_BUFSIZE, HWEPAR_BUFSTART);

#ifdef HWETRACE_PARSTAT
	HWETRACE_PRINT("Parallel statistics enabled.\n");
	tracer_wait = 0;
	tracer_sync = 0;
	tracer_buff = 0;
	tracer_sign = 0;
#endif
	

	//launch handler's thread
	pthread_create(&hwepar->handler, NULL, hwepar_thread, hwepar);
}


/*
 * get a clean container from the handler
 */
static inline hwe_cont *
hwepar_alloc(struct hwepar *hwepar)
{
	hwe_cont *res;
#if HWEPAR_MODE == 0
	// is there some container available ?
	if (hwepar->empty_idx == HWEPAR_BUFSIZE) {
		//no, get some
		hwepar_switchbuffer(hwepar);
	}
	res = hwepar->array[hwepar->empty_idx++];
	DEBUG_DO(hwepar->array[hwepar->empty_idx-1] = NULL);
#elif HWEPAR_MODE == 1
	if (hwepar->empty_idx == HWEPAR_BUFSIZE - 1) {
		hwepar_switchbuffer(hwepar);
	}
	res = hwepar->array[++(hwepar->empty_idx)];
	DEBUG_DO(hwepar->array[hwepar->empty_idx] = NULL);
#elif HWEPAR_MODE == 2
	DEBUG_ASSERT(&hwepar->buf->array[0] <= hwepar->empty_ptr);
	DEBUG_ASSERT(hwepar->empty_ptr <= &hwepar->buf->array[HWEPAR_BUFSIZE]);
  	DEBUG_ASSERT(hwepar->end_ptr == &hwepar->buf->array[HWEPAR_BUFSIZE]);
	if (hwepar->empty_ptr == hwepar->end_ptr) {
		hwepar_switchbuffer(hwepar);
	}
	DEBUG_ASSERT(&hwepar->buf->array[0] <= hwepar->empty_ptr);
	DEBUG_ASSERT(hwepar->empty_ptr <= &hwepar->buf->array[HWEPAR_BUFSIZE-1]);
	DEBUG_ASSERT(*(hwepar->empty_ptr) != NULL);
	res = *(hwepar->empty_ptr++);
	DEBUG_DO(*(hwepar->empty_ptr-1) = NULL);
#elif HWEPAR_MODE == 3
	DEBUG_ASSERT(&hwepar->buf->array[-1] <= hwepar->empty_ptr);
	DEBUG_ASSERT(hwepar->empty_ptr <= &hwepar->buf->array[HWEPAR_BUFSIZE-1]);
  	DEBUG_ASSERT(hwepar->end_ptr == &hwepar->buf->array[HWEPAR_BUFSIZE-1]);
	if (hwepar->empty_ptr == hwepar->end_ptr) {
		hwepar_switchbuffer(hwepar);
	}
	DEBUG_ASSERT(&hwepar->buf->array[-1] <= hwepar->empty_ptr);
	DEBUG_ASSERT(hwepar->empty_ptr <= &hwepar->buf->array[HWEPAR_BUFSIZE-2]);
	DEBUG_ASSERT(*(hwepar->empty_ptr+1) != NULL);
	res = *(++hwepar->empty_ptr);
	DEBUG_DO(*(hwepar->empty_ptr) = NULL);
#endif
	DEBUG_ASSERT(res->common.self != HWE_REF_NULL);
	return res;
}


/*
 * send a full event to the handler
 */
static inline
void hwepar_handle_local(struct hwepar *hwepar, hwe_cont *cont)
{
#if HWEPAR_MODE == 0
	// is there some space into the current buffer ?
	if (hwepar->full_idx == hwepar->empty_idx) {
		//no, get some
		hwepar_switchbuffer(hwepar);
	}
	DEBUG_ASSERT(hwepar->array[hwepar->full_idx] == NULL);
	hwepar->array[hwepar->full_idx++] = cont;
#elif HWEPAR_MODE == 1
	if (hwepar->full_idx == hwepar->empty_idx) {
		hwepar_switchbuffer(hwepar);
	}
	DEBUG_ASSERT(hwepar->array[hwepar->full_idx+1] == NULL);
	hwepar->array[++(hwepar->full_idx)] = cont;
#elif HWEPAR_MODE == 2
	DEBUG_ASSERT(&hwepar->buf->array[0] <= hwepar->full_ptr);
  	DEBUG_ASSERT(hwepar->full_ptr <= hwepar->empty_ptr);
	DEBUG_ASSERT(hwepar->empty_ptr <= &hwepar->buf->array[HWEPAR_BUFSIZE]);
	if (hwepar->full_ptr == hwepar->empty_ptr) {
		hwepar_switchbuffer(hwepar);
	}
	DEBUG_ASSERT(&hwepar->buf->array[0] <= hwepar->full_ptr);
	DEBUG_ASSERT(hwepar->full_ptr <= &hwepar->buf->array[HWEPAR_BUFSIZE - 1]);
	DEBUG_ASSERT(*(hwepar->full_ptr) == NULL);
	*(hwepar->full_ptr++) = cont;
#elif HWEPAR_MODE == 3
	DEBUG_ASSERT(&hwepar->buf->array[-1] <= hwepar->full_ptr);
  	DEBUG_ASSERT(hwepar->full_ptr <= hwepar->empty_ptr);
	DEBUG_ASSERT(hwepar->empty_ptr <= &hwepar->buf->array[HWEPAR_BUFSIZE-1]);
	if (hwepar->full_ptr == hwepar->empty_ptr) {
		hwepar_switchbuffer(hwepar);
	}
	DEBUG_ASSERT(&hwepar->buf->array[-1] <= hwepar->full_ptr);
	DEBUG_ASSERT(hwepar->full_ptr <= &hwepar->buf->array[HWEPAR_BUFSIZE - 2]);
	DEBUG_ASSERT(*(hwepar->full_ptr+1) == NULL);
	*(++hwepar->full_ptr) = cont;
#endif
}
static inline
void hwepar_handle(struct hwepar *hwepar, hwe_cont *cont)
{
	DEBUG_ASSERT(cont != NULL);
	hwepar_handle_local(hwepar, cont);
}

/*
 * stop
 */
static void
hwepar_stop(struct hwepar *hwepar) {
	//send a NULL job to tell it is the end
	hwepar_handle_local(hwepar, NULL);
	//flush the current buffer
	hwepar_switchbuffer(hwepar);
	pthread_mutex_lock(&hwepar->tracer_mutex);
	pthread_cond_signal(&hwepar->tracer_cond);	
	pthread_mutex_unlock(&hwepar->tracer_mutex);
	//join with the handler
	pthread_join(hwepar->handler, NULL);
	//free the buffers
	free(hwepar->alloc);

#ifdef HWETRACE_PARSTAT
	HWETRACE_PRINT("Tracer's sync stats: sync/sign/wait %"PRIu64"/%"PRIu64"/%"PRIu64", bps %.2f\n",
			tracer_sync, tracer_sign, tracer_wait,
			(0.0 + tracer_buff) / tracer_sync);
#endif
}

/*
 * get a new buffer
 */
static inline bool
hwepar_tracer_sync(struct hwepar *hwepar)
{
	unsigned hdler_idx = hwepar->handler_idx;
	hwepar->bufrdy = (hdler_idx + HWEPAR_NBBUF - hwepar->bufidx) % HWEPAR_NBBUF;
	return (hwepar->bufrdy > 0);
}
static inline void
hwepar_newbuffer(struct hwepar *hwepar)
{
	//update shared index and get the new buffer
	hwepar->tracer_idx = hwepar->bufidx;
	hwepar->buf = &hwepar->buffers[hwepar->bufidx].buffer;

	//set local variable
	//we have to get a buffer with a full_idx = 0 and empty_idx = BUFSTART
#if HWEPAR_MODE == 0
	hwepar->array = &hwepar->buf->array[0];
	hwepar->empty_idx = HWEPAR_BUFSTART;
	hwepar->full_idx = 0;
#elif HWEPAR_MODE == 1
	hwepar->array = &hwepar->buf->array[0];
	hwepar->empty_idx = HWEPAR_BUFSTART - 1;
	hwepar->full_idx = -1;
#elif HWEPAR_MODE == 2
	hwe_cont **array  = (hwe_cont **) &hwepar->buf->array[0];
	hwepar->full_ptr  = &array[0];
	hwepar->empty_ptr = &array[HWEPAR_BUFSTART];
	hwepar->end_ptr   = &array[HWEPAR_BUFSIZE];
#elif HWEPAR_MODE == 3
	hwe_cont **array  = (hwe_cont **) &hwepar->buf->array[-1];
	hwepar->full_ptr  = &array[0];
	hwepar->empty_ptr = &array[HWEPAR_BUFSTART];
	hwepar->end_ptr   = &array[HWEPAR_BUFSIZE];
#endif
}
static inline void
hwepar_switchbuffer(struct hwepar *hwepar)
{
	//set buffer info
#if HWEPAR_MODE <= 1
	hwepar->buf->empty_idx = hwepar->empty_idx;
	hwepar->buf->full_idx = hwepar->full_idx;
#else
	hwepar->buf->empty = (hwe_cont **) hwepar->empty_ptr;
	hwepar->buf->full  = (hwe_cont **) hwepar->full_ptr;
#endif
	TRACER_VPRINT("switch => [%u]%lu/%lu/%d\n", hwepar->bufidx, 
			FULL_IDX(hwepar->buf), EMPTY_IDX(hwepar->buf), HWEPAR_BUFSIZE);

	//increment the buffer index
	hwepar->bufidx = (hwepar->bufidx + 1) % HWEPAR_NBBUF;

	//we eventually wait to really have the new buffer 
	//before signaling our change to ensure the all buffers 
	//will not be dirty
	//this is needed because of the tracer_idx == handler_idx dilemma
	//this way there is no dilemma: it means all the buffers are clean
	
	//do we know we can switch safely to the next one ?
	if (--hwepar->bufrdy > 0) {
		hwepar_newbuffer(hwepar);
		return;
	}

	// no we need to synchronize
#ifdef HWETRACE_PARSTAT
	tracer_sync += 1;
#endif
	if (hwepar_tracer_sync(hwepar)) {
#ifdef HWETRACE_PARSTAT
		tracer_buff += hwepar->bufrdy;
#endif
		if (hwepar->bufrdy < (HWEPAR_NBBUF / 2)) {
			// signal here but with no mutex
			pthread_cond_signal(&hwepar->tracer_cond);	
#ifdef HWETRACE_PARSTAT
		tracer_sign += 1;
#endif
		}
		TRACER_VPRINT("sync => %u buffers\n", hwepar->bufrdy);
		hwepar_newbuffer(hwepar);
		return;
	}
	
	// still no, we need to wait
#ifdef HWETRACE_PARSTAT
	tracer_wait += 1;
#endif
	// signal here with mutex protection to avoid deadlock:
	//   if we are to wait for the condition to be signaled
	//   then the handler has job to do
	pthread_mutex_lock(&hwepar->tracer_mutex);
	pthread_cond_signal(&hwepar->tracer_cond);	
	pthread_mutex_unlock(&hwepar->tracer_mutex);

	//wait	
	do {
		pthread_mutex_lock(&hwepar->handler_mutex);
		if (!hwepar_tracer_sync(hwepar)) {
			pthread_cond_wait(&hwepar->handler_cond, &hwepar->handler_mutex);
		}
		pthread_mutex_unlock(&hwepar->handler_mutex);
	} while (!hwepar_tracer_sync(hwepar));
	
	//finally get the buffer	
#ifdef HWETRACE_PARSTAT
	tracer_buff += hwepar->bufrdy;
#endif
	TRACER_VPRINT("wait => %u buffers\n", hwepar->bufrdy);
	hwepar_newbuffer(hwepar);
}

static inline unsigned hwepar_handler_sync(struct hwepar *hwepar, unsigned bufidx)
{
	return (hwepar->tracer_idx + HWEPAR_NBBUF - bufidx) % HWEPAR_NBBUF;
}
static void* hwepar_thread(void *arg) {
	struct hwepar *hwepar = arg;
	
	unsigned bufidx = hwepar->handler_idx;
	union hwepar_buffer_align *buffers = hwepar->buffers;

	/*
	 * time variables which are needed to predict the moment when
	 * some buffers will be ready for processing
	 */
	struct timespec lastsync;
	long slot = 1000000000;
	clock_gettime(CLOCK_REALTIME, &lastsync);

	/*
	 * stat variables
	 */
#ifdef HWETRACE_PARSTAT
	int emin = HWEPAR_BUFSIZE, emax = 0, emoy = 0;
	int fmin = HWEPAR_BUFSIZE, fmax = 0, fmoy = 0;
	uint64_t nbev = 0, nbloop = 0, nbwait = 0, nbsign = 0, nbtout = 0, nbbuf = 0;
#endif
	
	bool eot = false;//end of trace flag
	while (!eot) {
#ifdef HWETRACE_PARSTAT
		nbloop += 1;
#endif
		
		/*
		 * set in _time_ the last moment when we got buffers
		 */
		struct timespec time = lastsync;

		/*
		 * do we need to wait ?
		 */
		unsigned todo = hwepar_handler_sync(hwepar, bufidx);
		if (todo == 0) {
			/*
			 * loop until we've got buffers to work on
			 */
#ifdef HWETRACE_PARSTAT
			unsigned haswaited = 0;
#endif
			do {

				/*
				 * compute the time we will wait before retrying
				 * by adding some time to the last moment
				 */
				time.tv_nsec += slot;
				if (time.tv_nsec > 1000000000) {
					time.tv_nsec -= 1000000000;
					time.tv_sec += 1;
				}
				slot *= 2;//double _slot_ to avoid looping too much
				if (slot > 1000000000) {
					slot = 1000000000;
				}
			
				/*
				 * eventually (we recheck with the mutex) wait
				 */
				pthread_mutex_lock(&hwepar->tracer_mutex);
				if ((todo = hwepar_handler_sync(hwepar, bufidx)) == 0) {
					HANDLER_VPRINT("wait until %.6f\n", 
							(0.0 + time.tv_sec) + (0.0 + time.tv_nsec) / 1000000000);
#ifdef HWETRACE_PARSTAT
					int ret = 
#endif
					pthread_cond_timedwait(&hwepar->tracer_cond, &hwepar->tracer_mutex, &time);
#ifdef HWETRACE_PARSTAT
					haswaited = 1;
					if (ret == 0) //normal termination (ie due to a _signal_ call)
						nbsign += 1;
					else //timedout
						nbtout += 1;
#endif
				}
				pthread_mutex_unlock(&hwepar->tracer_mutex);
			} while (todo == 0);
#ifdef HWETRACE_PARSTAT
			nbwait += haswaited;
#endif
		}

		/*
		 * register the current time and the time it taked to get the buffers
		 * in order to predict the time we will have to wait 
		 */
		clock_gettime(CLOCK_REALTIME, &time);
		slot = (time.tv_sec - lastsync.tv_sec) * 1000000000 + time.tv_nsec - lastsync.tv_nsec;
		lastsync = time;
		// at this point the slot time corresponds to _todo_ buffers
		// we make it correspond to 2 buffers in order to expect having at least 1
		// the next time.
		slot = (slot * 2) / todo;
		if (slot <= 0 || slot > 1000000000) {
			slot = 1000000000;
		}

		/*
		 * we have _todo_ buffer to work on
		 */
		HANDLER_VPRINT(" Got %u buffers. Slot time is now %.0f\n", todo, (0.0 + slot) / 1000);
		do {
			/*
			 * get the next buffer
			 */
			volatile struct hwepar_buffer *buf = &buffers[bufidx].buffer;
			const int fidx = FULL_IDX(buf);
			const int eidx = EMPTY_IDX(buf);
			HANDLER_VPRINT("Working on [%u]%d/%d/%d\n", 
					bufidx, fidx, eidx, HWEPAR_BUFSIZE);
#ifdef HWETRACE_PARSTAT
			nbbuf++;
			nbev += fidx;
#endif

			/*
			 * handle received events
			 */
			for (int i = 0; i < fidx; i++) {
				hwe_cont *cont = buf->array[i];
				if (cont == NULL) {
					/*
					 * this is the end
					 */
					HANDLER_VPRINT("=> EOT at index %d\n", i);
					eot = true;
					break;
				}
				handle_event(cont);
			}

			/*
			 * check the array
			 */
#ifdef HWEPAR_DEBUG
			for (int i = 0; i < fidx; i++) {
				buf->array[i] = NULL;
			}
			for (int i = fidx; i < eidx; i++) {
				DEBUG_ASSERT(buf->array[i] == NULL);
			}
			for (int i = eidx; i < HWEPAR_BUFSIZE; i++) {
				DEBUG_ASSERT(buf->array[i] != NULL);
			}
#endif
			
			/*
			 * the end ?
			 */
			if (eot)
				break;

			/*
			 * replace the used empty containers
			 */
			for (int i = HWEPAR_BUFSTART; i < eidx; i++) {
				hwe_cont *cont = handle_alloc();
				hwe_head_init(&cont->common);
				buf->array[i] = cont;
			}

			/*
			 * register buffer usage for stats purpose
			 */
#ifdef HWETRACE_PARSTAT
			if (nbbuf > HWEPAR_NBBUF) {
				//the first NBBUF buffers are discarded since they are empty
				fmoy += fidx;
				if (fidx < fmin)
					fmin = fidx;
				if (fidx > fmax)
					fmax = fidx;
				emoy += eidx;
				if (eidx < emin)
					emin = eidx;
				if (eidx > emax)
					emax = eidx;
			}
#endif


			/*
			 * Reset buffer index.
			 * Note: it use useless, the tracer will not read them 
			 */
#ifdef HWEPAR_DEBUG	
#if HWEPAR_MODE == 0
			buf->full_idx = 0;
			buf->empty_idx = HWEPAR_BUFSTART;
#elif HWEPAR_MODE == 1
			buf->full_idx = 0;
			buf->empty_idx = HWEPAR_BUFSTART;
#elif HWEPAR_MODE == 2
			buf->full = (hwe_cont **) &buf->array[0];
			buf->empty = (hwe_cont **) &buf->array[HWEPAR_BUFSTART];
#elif HWEPAR_MODE == 3
			buf->full = (hwe_cont **) &buf->array[-1];
			buf->empty = (hwe_cont **) &buf->array[HWEPAR_BUFSTART-1];
#endif
#endif

			/*
			 * switch to the next buffer (and signal it)
			 */
			bufidx = (bufidx + 1) % HWEPAR_NBBUF;
			pthread_mutex_lock(&hwepar->handler_mutex);
			hwepar->handler_idx = bufidx;
			pthread_cond_signal(&hwepar->handler_cond);
			pthread_mutex_unlock(&hwepar->handler_mutex);

		} while (--todo > 0);
	}

	/*
	 * print stats
	 */
#ifdef HWETRACE_PARSTAT
	HWETRACE_PRINT("Transmitted %"PRIu64" events in %"PRId64" buffers\n", nbev, nbbuf - HWEPAR_NBBUF);
	HWETRACE_PRINT("Buffer usage (min/avg/max): empty %u/%.2f/%u full %u/%.2f/%u\n",
			emin, (0.0 + emoy) / (nbbuf - HWEPAR_NBBUF), emax,
			fmin, (0.0 + fmoy) / (nbbuf - HWEPAR_NBBUF), fmax);
	HWETRACE_PRINT("Handler's sync stats: %"PRIu64" loops (%.2f bpl),"
			" %"PRIu64":%"PRIu64"/%"PRIu64" wait:sign/tout\n",
			nbloop, (0.0 + nbbuf) / nbloop,
			nbwait, nbsign, nbtout);
#endif

	return NULL;
}

#endif
