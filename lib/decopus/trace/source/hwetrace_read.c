
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>

#include "events/hwe_tools.h"
#include "hwe_handle.h"

typedef struct rhwe_t rhwe_t;
typedef struct comp_t comp_t;
typedef struct ref_t ref_t;

#ifdef RHWE_VERBOSE
#define RHWE_CHECK
static unsigned vlvl = 0;
#include "alloca.h"

#define RHWE_VCALL(fmt, ...) { \
   for (unsigned i = 0; i < vlvl; i++) \
      fprintf(stdout, " "); \
   fprintf(stdout, "<%s> " fmt "\n", __func__, ##__VA_ARGS__); \
   vlvl++; \
}

#define RHWE_VCALL_EV(ev) { \
   char *str = alloca(40); \
   hwe_desc(ev, ref2id, str, 40); \
   str[39] = '\0'; \
   RHWE_VCALL("%s", str); \
}

#define RHWE_VRET { \
   vlvl--; \
}

#else
#define RHWE_VCALL(fmt, ...)
#define RHWE_VCALL_EV(ev)
#define RHWE_VRET 
#endif

// node for building a linked list of event waiting for another one
struct ref_t {
   ref_t    *next;
   rhwe_t   *rhwe; // waiting node
   hwe_cont *cont; // ptr to the ref container
   unsigned  pos;  // ref index in the array [0;HWE_REF_MAX[
};

// pool for ref_t
#ifdef RHWE_CHECK
#define POOL_INIT
#endif
#define POOL_grain 100
#define POOL_prefix ref_pool
#define POOL_elem_t ref_t
#include "pool.h"

struct id_array {
	hwe_id_t ids[HWE_REF_MAX];
};
#define POOL_prefix ids_pool
#define POOL_elem_t struct id_array
#define POOL_grain 100
#include "pool.h"

// in this file


// structure representing an event to be in a linked list
struct rhwe_t {
   hwe_cont *hwe;  // ptr to the event
   comp_t   *comp;  // src component 
   rhwe_t   *next;  // next in component linked list
   ref_t    *waiting; // list of waiting nodes (for ref)
   int       state;//0:unread; 1:read; 2;commited
   int       miss_ref;//count of remaining ref ptr to solve for this event (before commit)
   int       miss_exp;//number of remaining ref ptr to this event to solve (before delete)
};

// tree of rhwe_t
#define AVL_key_t  hwe_id_ind_t
#define AVL_data_t rhwe_t
#define AVL_prefix rhwe
#include "avl.h"

// component
// its events are in a tree (for finding them) and in linked list (for the order)
struct comp_t {
   hwe_id_dev_t id;    // id of component
   rhwe_tree_t  tree;  // avl of event
};
#define BUFSIZE ((size_t)1000000U)


// main fifo
struct mainstruct {
   //pools
   rhwe_pool_t rhwe_pool;
   ref_pool_t  ref_pool;
	ids_pool_t  ids_pool;
   //file & buffer
   int    fd; //file descriptor
   void  *buf;//first byte of the buffer
   void  *bcur;//current position in the buffer
   size_t bsize;//number of byte in the buffer starting from current position
   //components arrays
   unsigned      ncomp;//size of the arrays
   comp_t      **comp;//array of pointers to every component 
   hwe_id_ind_t *previd;//array containing the id of the last event for each component
   //fifo
   rhwe_t *fifo_head;
   rhwe_t *fifo_queue;
   //stat
   unsigned long long nread;
};

/*********
 * UTILS *
 *********/
__attribute__((__unused__))
static hwe_id_t ref2id(hwe_ref_t ref)
{
	hwe_id_t id = HWE_ID_NULL;
	return id;
}

/*
 * get corresponding node ptr
 */
static rhwe_node_t *getnode(rhwe_t *rhwe)
{
   void *ptr = rhwe;
   ptr -= ((void*) &(((rhwe_node_t*)NULL)->data)) - NULL;
   return ptr;
}

/*
 * alloc an init a device structure
 */
static comp_t *alloc_comp(struct mainstruct *ms, unsigned id)
{
   comp_t *res = malloc(sizeof(*res));
   res->id = id;
   rhwe_init(&res->tree, &ms->rhwe_pool);
   return res;
}

/*
 * find a node
 * eventually create it
 */
static rhwe_node_t *findnode(comp_t *c, hwe_id_ind_t id)
{
   rhwe_node_t *node = NULL;
   if (rhwe_add(&c->tree, id, &node)) {
      //node is added
      node->data.comp = c;
      node->data.hwe  = NULL;
      node->data.next = NULL;
      node->data.state = 0;
      node->data.miss_ref = 0;
      node->data.miss_exp = 0;
      node->data.waiting = NULL;
      RHWE_VCALL("create %u.%u[%d;%d;%d] ", c->id, id,
                  node->data.state,
                  node->data.miss_ref,
                  node->data.miss_exp);
   } else {
      RHWE_VCALL("%u.%u[%d;%d;%d] ", c->id, id,
                  node->data.state,
                  node->data.miss_ref,
                  node->data.miss_exp);
   }
   RHWE_VRET;
   return node;
}

/********
 * MAIN *
 ********/
static bool read_container(struct mainstruct *ms, hwe_cont *cont, struct id_array *ids);
static void update_previd(struct mainstruct *ms, hwe_id_t id);
static void try_solve_our_ref(struct mainstruct *ms, rhwe_t *rhwe);
static void solve_pending_ref(struct mainstruct *ms, rhwe_t *rhwe);
static rhwe_t *insert_event(struct mainstruct *ms, hwe_cont *hwe);
static bool try_commit(struct mainstruct *ms, rhwe_t *rhwe);
static bool try_delete(rhwe_t *rhwe);
int main (int argc, char **argv)
{
   struct mainstruct ms;
   
   if (argc < 2) {
      fprintf(stderr, "usage: %s <tracefilename> [...]\n", argv[0]);
      exit(1);
   }

   /*
    *  initializing
    */
   // file and buffer
   ms.fd = open(argv[1], O_RDONLY);
   if (ms.fd < 0) {
      fprintf(stderr, "failed to open `%s'\n", argv[1]);
      exit(1);
   }
   ms.buf = malloc(BUFSIZE);
   ms.bcur = ms.buf;
   ms.bsize = 0;
   //fifo
   ms.fifo_head = NULL;
   ms.fifo_queue = NULL;
   // init event pools
   rhwe_pool_init(&ms.rhwe_pool);
   ref_pool_init(&ms.ref_pool);
   ids_pool_init(&ms.ids_pool);
   // device array
   ms.ncomp = 16;
   ms.previd = calloc(ms.ncomp, sizeof(hwe_id_ind_t));
   ms.comp = malloc(sizeof(comp_t *) * ms.ncomp);
   for (unsigned i = 0; i < ms.ncomp; i += 1)
      ms.comp[i] = alloc_comp(&ms, i);
   //stat
   ms.nread = 0;
 
   // handle module start 
   handle_start(argv[1], argc - 2, (argc > 2)?&argv[2]:NULL);


   /*
    * fetch events one by one
    */
   hwe_cont *current = NULL;//current event
   do {
      hwe_cont *next = NULL;//will store the first container of the next event  
     
      /*
       * complete current event until it is full (may be several container)
       * (in fact we do not know it is full until we've find a new one)
       */
      if (current != NULL)
         current->common.reflast = &current->common;//init list's queue
      do {
         
         /*
          * read one 'true' container (skipping HWE_ID)
          */
         bool stop = false; //use to indicate an end of file
         hwe_cont *cont = handle_alloc();
#ifdef RHWE_CHECK
         memset(cont, 0, sizeof(hwe_cont));
#endif
			struct id_array *ids = ids_pool_get(&ms.ids_pool);
        //init container's pointers
         cont->common.reflast = NULL;
         cont->common.refnext = NULL;
         do {//loop until we find a non-HWE_ID

            // read one container from the file
            if (!read_container(&ms, cont, ids)) {
               stop = true;
               break;
            }

            // update previd array (and eventually increase ncomp)
            update_previd(&ms, cont->common.id);

            //hwe_print(stdout, cont, NULL, "");//debug
         } while (cont->common.head.type == HWE_ID);
         if (stop)
            break;

			// register (hwe_id_t *) into refs array
			cont->common.refs[0] = (hwe_ref_t) ids;

         if (current != NULL &&
             current->common.id.devid == cont->common.id.devid &&
             current->common.id.index == cont->common.id.index) {
            //the container is part of the current event
            current->common.reflast->refnext = &cont->common;
            current->common.reflast = &cont->common; 
         } else {
            next = cont;
         }

      } while (next == NULL);

      /*
       * handle current event
       * (during the first iteration of the loop current is NULL)
       */
      if (current != NULL) {
         ms.nread += 1;
         rhwe_t *rhwe;//internal way to reprensent the event
      
         //Create event/add it in component's sets
         rhwe = insert_event(&ms, current);
      
         //try to solve references of our event (ie: convert ids into pointers)
         try_solve_our_ref(&ms, rhwe);

         //resolve pending references to our event
         solve_pending_ref(&ms, rhwe);

         //mark this event as 'read' and try to commit it
         rhwe->state = 1;//only do this now to ensure it will not be commited before
         try_commit(&ms, rhwe);
      }

      // switch the next event
      current = next;
   } while (current != NULL);

   /*
    * closing
    */
   fprintf(stderr, "Event read : %lu\n", (unsigned long) ms.nread);
   handle_stop();
   close(ms.fd);
   rhwe_pool_term(&ms.rhwe_pool);
   ref_pool_free(&ms.ref_pool);
   for (unsigned i = 0; i < ms.ncomp; i++)
      free(ms.comp[i]);
   free(ms.comp);
   free(ms.previd);
}


/*****************************************
 * Insert an event into a component sets *
 *****************************************/
static rhwe_t* insert_event(struct mainstruct *ms, hwe_cont *hwe)
{
   RHWE_VCALL_EV(hwe);
   
   //get the component
   comp_t *c = ms->comp[hwe->common.id.devid];

   //get the event node and update it
   rhwe_node_t *node = findnode(c, hwe->common.id.index);
   rhwe_t *rhwe = &node->data;
   rhwe->hwe = hwe;

   //add into fifo (ensure the order is preserved)
   if (ms->fifo_queue)
      ms->fifo_queue->next = rhwe;
   else
      ms->fifo_head = rhwe;
   ms->fifo_queue = rhwe;

   // this event is referenced and we need to wait the references
   rhwe->miss_exp += hwe->common.head.expected;
   
   RHWE_VRET;
   //return corresponding base event
   return rhwe;
}

/**********************************
 * read a container from the file *
 **********************************/
static bool read_container(struct mainstruct *ms, hwe_cont *cont, struct id_array *ids)
{
   size_t used = hwe_read(cont, &ids->ids[0], ms->bcur, ms->bsize, ms->previd, ms->ncomp);
   if (used == 0) {
      /*
       * buffer does not contain enough data
       * We need to refill it
       */
      //fprintf(stderr, "refill buffer (still %u bytes)\n", (unsigned int) bsize);

      //buffer should be almost empty so memcpy and not memove
      ms->bcur = memcpy(ms->buf, ms->bcur, ms->bsize);
      ssize_t r = read(ms->fd, ms->buf + ms->bsize, BUFSIZE - ms->bsize);
      if (r <= 0) {
         if (r < 0) {
            fprintf(stderr, "Error when trying to read from trace file\n");
            exit(1);
         }
         return false;
      }
      ms->bsize += r;
      used = hwe_read(cont, &ids->ids[0], ms->bcur, ms->bsize, ms->previd, ms->ncomp);
      if (used == 0) {
         fprintf(stderr, "Internal Error: couldn't read event, need a bigger buffer\n");
         exit(1);
      }
   }
   ms->bcur += used;
   ms->bsize -= used;

	/*
	 * set id refs
	 */
   return true;
}

/****************************
 * update compoennts arrays *
 ****************************/
static void update_previd(struct mainstruct *ms, hwe_id_t id)
{
   if (ms->ncomp <= id.devid) {
      unsigned nold = ms->ncomp;
      ms->ncomp *= 2;
      
      ms->previd = realloc(ms->previd, sizeof(hwe_id_ind_t) * ms->ncomp);
      ms->comp = realloc(ms->comp, sizeof(comp_t *) * ms->ncomp);
      
      for (unsigned i = nold; i < ms->ncomp; i += 1) {
         ms->comp[i] = alloc_comp(ms, i);
         ms->previd[i] = 0;
      }
   }
   ms->previd[id.devid] = id.index;
}

/************************
 * solve our references *
 ************************/
/* (convert id into pointer if possible,
 *                   register the conversion for later if not possible)
 */
static void try_solve_our_ref(struct mainstruct *ms, rhwe_t *rhwe)
{
   //loop on each container of the event
   for (hwe_cont *cont = rhwe->hwe; cont != NULL; cont = (hwe_cont *) cont->common.refnext) {
      //loop on on each reference
		struct id_array *ids = (struct id_array *) cont->common.refs[0];
      for (unsigned i = 0; i < cont->common.head.nrefs; i += 1) {
         hwe_id_t refid = ids->ids[i];
#ifdef RHWE_CHECK
         if (refid.devid >= ms->ncomp || !ms->comp[refid.devid]) {
            hwe_print(stderr, cont, ref2id, "<ERROR> ", NULL);
            fprintf(stderr, "Error: reference to a non existing component: %u\n", refid.devid);
            exit(EXIT_FAILURE);
         }
#endif
         //search the referenced event
         rhwe_node_t *node = findnode(ms->comp[refid.devid], refid.index);

         if (node->data.hwe) {//event has already been read

            //set ptr to the referenced event
            hwe_setref(&cont->common, i, node->data.hwe->common.self);

            //update expected reference counter and try delete it
            node->data.miss_exp -= 1;
            try_delete(&node->data);

         } else {//unread event
            //add a ref in its waiting list
            ref_t *ref = ref_pool_get(&ms->ref_pool);
            ref->rhwe = rhwe;
            ref->cont = cont;
            ref->pos = i;
            ref->next = node->data.waiting;
            node->data.waiting = ref;
            rhwe->miss_ref += 1;// waiting resolution
            hwe_setref(&cont->common, i, HWE_REF_NULL);
         }
      }
		ids_pool_put(&ms->ids_pool, ids);
   }
}

/****************************************
 * solve pending reference to our event *
 ****************************************/
static void solve_pending_ref(struct mainstruct *ms, rhwe_t *rhwe)
{
   while (rhwe->waiting) {
      //solve a ref
      ref_t *ref = rhwe->waiting;
      hwe_setref(&ref->cont->common, ref->pos, rhwe->hwe->common.self);

      //update our expected reference counter
      rhwe->miss_exp -= 1;

      //update referencing event's pending counter and try to commit it
      ref->rhwe->miss_ref -= 1;
      try_commit(ms, ref->rhwe);
      
      //update pending list
      rhwe->waiting = ref->next;
      // put ref back in the pool
      ref_pool_put(&ms->ref_pool, ref);
   }
}

/**************************
 * Try to commit an event *
 **************************/
// if it is in head of the queue and all it's ref ptr have been solved
static bool try_commit(struct mainstruct *ms, rhwe_t *rhwe)
{
   RHWE_VCALL_EV(rhwe->hwe);
   if (ms->fifo_head != rhwe) {
      RHWE_VRET;
      return false;
   }
   if (rhwe->state != 1 || rhwe->miss_ref != 0) {
      RHWE_VRET;
      return false;
   }
   do {
      RHWE_VCALL_EV(rhwe->hwe);

#ifdef RHWE_CHECK
      if (rhwe->waiting) {
         fprintf(stderr, "Error: Commiting a 'waited' event\n");
         exit(1);
      }
      for (hwe_cont *cur = rhwe->hwe; cur != NULL; cur = (hwe_cont *) cur->common.refnext) {
         for (unsigned i = 0; i < cur->common.head.nrefs; i += 1) {
            if (cur->common.refs[i] == HWE_REF_NULL) {
               fprintf(stderr, "Error: Commiting an event with a unresolved reference\n");
               exit(1);
            }
         }
      }
#endif
      //send event
      handle_event(rhwe->hwe);

      //switch to next event
      rhwe_t *tmp = rhwe;
      rhwe = rhwe->next;

      //delete event ?
      tmp->state = 2;
      try_delete(tmp);

      RHWE_VRET;
   } while (rhwe != NULL && rhwe->state == 1 && rhwe->miss_ref == 0);

   //update fifo's ptrs
   ms->fifo_head = rhwe;
   if (rhwe == NULL)
      ms->fifo_queue = NULL;

   RHWE_VRET;
   return true;
}

/************************************
 * Try to delete a node of an event *
 ************************************/
// if it has been commited and its ptr is not needed anymore
static bool try_delete(rhwe_t *rhwe)
{
   if (rhwe->state == 2 && rhwe->miss_exp == 0) {
      rhwe_rem(&rhwe->comp->tree, getnode(rhwe));
      return true;
   }
   return false;
}

