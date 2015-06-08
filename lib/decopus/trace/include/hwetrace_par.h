#ifndef _HWETRACE_PAR_H_
#define _HWETRACE_PAR_H_

#include <pthread.h>
#include "events/hwe_events.h"

// alignement of shared buffers (in bytes)
#ifndef HWEPAR_ALIGN
#define HWEPAR_ALIGN 64
#endif

// chunk's size
#ifndef HWEPAR_BUFGRAIN
#define HWEPAR_BUFGRAIN (HWEPAR_ALIGN / sizeof(hwe_cont*))
#endif

//nb of chunk in a buffer
#ifndef HWEPAR_BUFSIZE
#define HWEPAR_BUFSIZE 4
#endif

// the fifo structure
struct hwepar_fifo {
	char p0[HWEPAR_ALIGN];
	//read index and synchro
	volatile unsigned ridx;
	pthread_mutex_t rmutex;
	pthread_cond_t  rcond;
	//char p1[HWEPAR_ALIGN];
	//write index and synchro
	volatile unsigned widx;
	pthread_mutex_t wmutex;
	pthread_cond_t  wcond;
	char p2[HWEPAR_ALIGN];
	//circular buffer start
	hwe_cont **volatile buffer;
};

// local view of a fifo
struct hwepar_local {
	//current chunk index & remaining chunk cnt 
	unsigned idx;
	unsigned cnt;
	//base buffer
	hwe_cont **buf;
#ifdef HWEPAR_LOG
	unsigned log_next;
	unsigned log_sync;
	unsigned log_wait;
	unsigned log_sign;
	unsigned log_cond;
#endif
};

/*
 * init a fifo
 */
static inline void
hwepar_fifoinit(struct hwepar_fifo *fifo, void *buffer)
{
	// reader and writer are at index 0 (ie: the fifo is empty)
	fifo->ridx = 0;
	fifo->widx = 0;
	// align the buffer
	fifo->buffer = (void*)((((uintptr_t) buffer + (HWEPAR_ALIGN - 1)) / ((uintptr_t) HWEPAR_ALIGN)) * ((uintptr_t) HWEPAR_ALIGN));
	pthread_mutex_init(&fifo->rmutex, NULL);
	pthread_cond_init(&fifo->rcond, NULL);
	pthread_mutex_init(&fifo->wmutex, NULL);
	pthread_cond_init(&fifo->wcond, NULL);
}

/*
 * init a local struct of a fifo
 */
static void
hwepar_localfifoinit(struct hwepar_local *local, struct hwepar_fifo *fifo)
{
	local->cnt = 0;
	local->idx = 0;
	local->buf = fifo->buffer;
#ifdef HWEPAR_LOG
	local->log_next = 0;
	local->log_sync = 0;
	local->log_wait = 0;
	local->log_cond = 0;
	local->log_sign = 0;
#endif
}

/*
 * print log
 */
__attribute__((__unused__))
static void
hwepar_localfifolog(struct hwepar_local *local, const char *prefix)
{
#ifdef HWEPAR_LOG
	fprintf(stdout, "%s: next %u, sign %u, sync %u, wait %u, cond %u\n",
			prefix, local->log_next, local->log_sign, 
			local->log_sync, local->log_wait, local->log_cond);
#endif
}

/***************
 * FIFO WRITER *
 ***************/

/*
 * synchronize the local view with the fifo
 * return the number of chunk we can write into
 */
static unsigned
hwepar_fifowriter_sync(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
#ifdef HWEPAR_LOG
	local->log_sync++;
#endif
	// note: at this point we consider fifo->widx==local->idx
	//       thus a signal should have been done before if local->idx has changed
	// note: the fifo can't be full
	// so ridx==widx means the fifo is empty
	unsigned ridx = fifo->ridx;
	// compute free space between ridx and widx
	// we do + BUFSIZE in order to get a good result if BUFSIZE is not a power of 2
	// thanks to the "-1" we get the number of free chunk - 1
	//                and we get BUFSIZE-1 when ridx==widx
	// => so we can't fully fill the fifo
	local->cnt = ((ridx + HWEPAR_BUFSIZE - 1 - local->idx) % HWEPAR_BUFSIZE);
	return local->cnt;//in [0 and BUFSIZE-1]
}

/*
 * sync & wait that we can write into the current chunk
 */
__attribute__((__unused__))
static void
hwepar_fifowriter_wait(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
	// is there some safe space to read into ?
	if (local->cnt > 0) {
		return;
	}

	//sync to see if there is still no space
	if (hwepar_fifowriter_sync(fifo, local) > 0) {
		return;
	}
	
#ifdef HWEPAR_LOG
	local->log_wait++;
#endif
	//so, we really need to wait
	pthread_mutex_lock(&fifo->rmutex);
	while (hwepar_fifowriter_sync(fifo, local) <= 0) {
		pthread_cond_wait(&fifo->rcond, &fifo->rmutex);
#ifdef HWEPAR_LOG
		local->log_cond++;
#endif
	}
	pthread_mutex_unlock(&fifo->rmutex);
	return;
}

/*
 * return the current chunk (may not be safe to write into)
 */
static hwe_cont **
hwepar_fifowriter_current(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
	return &local->buf[local->idx * HWEPAR_BUFGRAIN];
}

/*
 * return true if the current chunk is ok
 */
__attribute__((__unused__))
static bool
hwepar_fifowriter_ok(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
	return (local->cnt > 0);
}

/*
 * switch to the next chunk
 * return true if we can write into it
 */
static bool
hwepar_fifowriter_next(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
#ifdef HWEPAR_LOG
	local->log_next++;
#endif
	//ok switch to next part of the buffer
	local->idx  = (local->idx + 1U) % HWEPAR_BUFSIZE;
	local->cnt -= 1;
	return (local->cnt > 0);
}

/*
 * signal our local changes to the fifo
 */
static void
hwepar_fifowriter_signal(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
#ifdef HWEPAR_LOG
	local->log_sign++;
#endif
	pthread_mutex_lock(&fifo->wmutex);
	fifo->widx = local->idx;
	pthread_cond_signal(&fifo->wcond);
	pthread_mutex_unlock(&fifo->wmutex);
}

/***************
 * FIFO GETTER *
 ***************/

/*
 * synchronize the local view with the fifo
 * return the number of chunk we can read into
 */
static unsigned
hwepar_fiforeader_sync(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
#ifdef HWEPAR_LOG
	local->log_sync++;
#endif
	// note: at this point we consider fifo->ridx==local->idx
	//       thus a signal should have been done before if local->idx has changed
	// note: the fifo can't be full
	// so ridx==widx means the fifo is empty
	unsigned widx = fifo->widx;
	//compute the number of element we can safely read
	// we do + BUFSIZE in order to get a good result if BUFSIZE is not a power of 2
	// we get 0 if widx==ridx
	local->cnt = (widx + HWEPAR_BUFSIZE - local->idx) % HWEPAR_BUFSIZE;
	return local->cnt;//in [0 and BUFSIZE-1]
}

/*
 * sync & wait that we can write into the current chunk
 */
__attribute__((__unused__))
static void
hwepar_fiforeader_wait(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
	// is there some safe space to read into ?
	if (local->cnt > 0) {
		return;
	}

	//sync to see if there is still no space
	if (hwepar_fiforeader_sync(fifo, local) > 0) {
		return;
	}
	
#ifdef HWEPAR_LOG
	local->log_wait++;
#endif
	//so, we really need to wait
	pthread_mutex_lock(&fifo->wmutex);
	while (hwepar_fiforeader_sync(fifo, local) <= 0) {
		pthread_cond_wait(&fifo->wcond, &fifo->wmutex);
#ifdef HWEPAR_LOG
		local->log_cond++;
#endif
	}
	pthread_mutex_unlock(&fifo->wmutex);
	return;
}

/*
 * return the current chunk (may not be safe to read into)
 */
static hwe_cont **
hwepar_fiforeader_current(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
	return &local->buf[local->idx * HWEPAR_BUFGRAIN];
}

/*
 * return true if the current chunk is ok
 */
__attribute__((__unused__))
static bool
hwepar_fiforeader_ok(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
	return (local->cnt > 0);
}


/*
 * switch to the next chunk
 * return true if we can read into it
 */
static bool
hwepar_fiforeader_next(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
#ifdef HWEPAR_LOG
	local->log_next++;
#endif
	//ok switch to next part of the buffer
	local->idx  = (local->idx + 1U) % HWEPAR_BUFSIZE;
	local->cnt -= 1;
	return (local->cnt > 0);
}

/*
 * signal our local changes to the fifo
 */
static void
hwepar_fiforeader_signal(struct hwepar_fifo *fifo, struct hwepar_local *local)
{
#ifdef HWEPAR_LOG
	local->log_sign++;
#endif
	pthread_mutex_lock(&fifo->rmutex);
	fifo->ridx = local->idx;
	pthread_cond_signal(&fifo->rcond);
	pthread_mutex_unlock(&fifo->rmutex);
}

#endif
