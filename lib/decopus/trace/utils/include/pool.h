
#include <stdlib.h>
#include <string.h>

/*
 * POOL_elem_t
 * type of the element stored in the pool
 */
#ifndef POOL_elem_t
#error "POOL_elem_t undefined"
#endif

/*
 * POOL_grain
 * number of elements allocated each time it is needed
 */
#ifndef POOL_grain
#error "POOL_grain undefined"
#endif

/* 
 * POOL_prefix
 * prefix for pool types and functions
 */
#ifndef POOL_prefix
#error "POOL_prefix undefined"
#endif

/*
 * POOL_INIT
 * enable memset(0) of an element before a get
 */

/*
 * POOL_DYNSIZE
 * enable the size of an element not being sizeof(POOL_elem_t)
 * it will be given at initialization
 */

/*
 * POOL_STAT
 * enable statistics (number of used and allocated elements)
 */

#define POOL_CONCAT(x,y) x ## _ ## y
#define POOL_XCONCAT(x,y) POOL_CONCAT(x,y)

#define POOL_t        POOL_XCONCAT(POOL_prefix, t)
#define POOL_array_t  POOL_XCONCAT(POOL_prefix, array_t)
#define POOL_union_t  POOL_XCONCAT(POOL_prefix, union_t)

#define POOL_init     POOL_XCONCAT(POOL_prefix, init)
#define POOL_free     POOL_XCONCAT(POOL_prefix, free)
#define POOL_get      POOL_XCONCAT(POOL_prefix, get)
#define POOL_put      POOL_XCONCAT(POOL_prefix, put)

typedef struct POOL_t POOL_t;
typedef struct POOL_array_t POOL_array_t;
typedef union POOL_union_t POOL_union_t;

union POOL_union_t {
   POOL_union_t *next;
   POOL_elem_t   elem;
};

/*
 * array of allocated elements
 */
struct POOL_array_t {
   POOL_array_t *next;
#ifdef POOL_DYNSIZE
   POOL_union_t elems[];
#else
   POOL_union_t elems[POOL_grain];
#endif
};

/*
 * pool structure
 */
struct POOL_t {
   POOL_union_t *list;     // linked list of free elements
   unsigned int  current;  // number of remaining elements in the head array
   POOL_array_t *alloc;    // linked list of allocated array
#ifdef POOL_DYNSIZE
   size_t elemsize;
   size_t arraysize;
#endif
#if defined(POOL_STAT) || defined(POOL_MAXELEM)
   unsigned int  nalloc;   // number of allocated elements
#endif
#ifdef POOL_STAT
   unsigned int  nused;    // nummber of currently used elements
#endif
};

/*
 * initialize a pool structure
 * eventually with the size of an element
 */
__attribute__((__unused__))
static void POOL_init (POOL_t *pool
#ifdef POOL_DYNSIZE
      ,size_t size
#endif
      )
{
   memset(pool, 0, sizeof(POOL_t));
#ifdef POOL_DYNSIZE
   pool->elemsize = size;
   pool->arraysize = sizeof(POOL_array_t) + size * POOL_grain;
#endif
}

/*
 * reset a pool structure (free all allocated arrays)
 */
__attribute__((__unused__))
static void POOL_free (POOL_t *pool)
{
   POOL_array_t *head = pool->alloc;
   while (head) {
      POOL_array_t *tmp = head;
      head = head->next;
      free(tmp);
   }
#ifdef POOL_DYNSIZE
   POOL_init(pool, pool->elemsize);
#else
   POOL_init(pool);
#endif
}

/*
 * get an element from the pool
 */
__attribute__((__unused__))
static POOL_elem_t * POOL_get (POOL_t *pool)
{
#ifdef POOL_SAFEMODE
#ifdef POOL_STAT
   pool->nused++;
#endif
#ifdef POOL_DYNSIZE
   return calloc(1, pool->elemsize);
#else
   return calloc(1, sizeof(POOL_elem_t));
#endif
#else

   POOL_union_t *res;
   if (pool->list) {
      res = pool->list;
      pool->list = pool->list->next;
   } else if (pool->current) {
#ifdef POOL_DYNSIZE
      void *ptr = &pool->alloc->elems[0];
      res = ptr + (pool->elemsize * (--(pool->current)));
#else
      res = &pool->alloc->elems[--(pool->current)];
#endif
#ifdef POOL_MAXELEM
	} else if (pool->nalloc >= POOL_MAXELEM) {
		return NULL;
#endif
   } else {
      POOL_array_t *array;
#ifdef POOL_DYNSIZE
      array = malloc(pool->arraysize);
#else
      array = malloc(sizeof(POOL_array_t));
#endif
      array->next = pool->alloc;
      pool->alloc = array;
      pool->current = POOL_grain - 1;
#ifdef POOL_DYNSIZE
      void *ptr = &array->elems[0];
      res = ptr + (pool->elemsize * (POOL_grain - 1));
#else
      res = &array->elems[POOL_grain - 1];
#endif
#if defined(POOL_STAT) || defined(POOL_MAXELEM)
      pool->nalloc += POOL_grain;
#endif
   }
#ifdef POOL_INIT
#ifdef POOL_DYNSIZE
   memset(res, 0, pool->elemsize);
#else
   memset(res, 0, sizeof(POOL_elem_t));
#endif
#endif

#ifdef POOL_STAT
   pool->nused++;
#endif
   return &res->elem;

#endif
}

/*
 * put back an element in the pool
 */
__attribute__((__unused__))
static void POOL_put (POOL_t *pool, POOL_elem_t *elem)
{
#ifdef POOL_SAFEMODE
   free(elem);
#else
   POOL_union_t *ulm = (POOL_union_t *) elem;
   ulm->next = pool->list;
   pool->list = ulm;
#endif
#ifdef POOL_STAT
   pool->nused--;
#endif
}

#undef POOL_CONCAT
#undef POOL_XCONCAT

#undef POOL_grain
#undef POOL_prefix

#undef POOL_elem_t
#undef POOL_t
#undef POOL_array_t
#undef POOL_union_t

#undef POOL_init
#undef POOL_free
#undef POOL_get
#undef POOL_put

#ifdef POOL_STAT
#undef POOL_STAT
#endif

#ifdef POOL_INIT
#undef POOL_INIT
#endif

#ifdef POOL_DYNSIZE
#undef POOL_DYNSIZE
#endif

#ifdef POOL_MAXELEM
#undef POOL_MAXELEM
#endif
