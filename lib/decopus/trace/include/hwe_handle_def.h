#ifndef _HWE_HANDLE_DEF_H_
#define _HWE_HANDLE_DEF_H_

/*
 * main types, represent events and components
 */
typedef struct comp_t comp_t;
typedef struct event_t event_t;

/*
 * type which represent a stage of the processing 
 */
enum stage_t {
   STAGE_TRACE,
   STAGE_INIT,
   STAGE_GO,
   STAGE_RET
};

/*
 * type for fifo of events
 */
typedef struct evfifo_t evfifo_t;
struct evfifo_t {
   //last event of the fifo
   event_t *queue;
   //indication of the queue stage which is in the fifo
   enum stage_t stage;
};

/*
 * element for linked list of event or fifo membership
 */
typedef struct evptr_t evptr_t;
struct evptr_t {
   //next element in the list
   evptr_t *next;
   //the blocked event
   event_t *event;
   //blocked stage of the event
   enum stage_t stage;
   //used when event==NULL
   //to indicate the membership in a given fifo
   evfifo_t *fifo;
};

#endif//_HWE_HANDLE_DEF_H_
