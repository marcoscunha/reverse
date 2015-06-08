#ifndef _HWE_COMMON_H_
#define _HWE_COMMON_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * Common definition for all events
 */

/*
 * version of trace (present in HWE_INFO event)
 */
#define HWE_VERSION_MAJOR 1
#define HWE_VERSION_MINOR 3
#define HWE_VERSION (256*HWE_VERSION_MAJOR + HWE_VERSION_MINOR)

/**********************************************************************
 ********     HWE_HEADER     ******************************************
 **********************************************************************/

/*
 * Common header for every event.
 *
 * Mapping:
 *    hwe_id_dev_t devid;
 *    hwe_head_t   head;
 *    hwe_date_t   dates[0..HWE_DATE_MAX];
 *    hwe_id_t     nrefs[0..HWE_REF_MAX];
 */

/*
 * Dependencies/References rules:
 * There is always 2 events involved:
 *  - the 'referenced' event
 *  - the 'referencing' event (which contains the id of the 'referenced' event)
 *
 * There is rules for dependencies contained into an event.
 * An event cannot depend on any event, there is some constraints.
 * All rules are here to bound the "life" of each event:
 * If we do not know if an event will still be referenced, we can't close/delete/process it.
 * 
 * There 2 types of dependencies/references: "expected" and "local".
 *
 * * Expected dependencies:
 * 
 * Some events expect to be referenced by others (ex: a memory access).
 * For each event the number of expected reference is known when reading the 
 * event, this allows to 'close' an event when all 'expected' references have 
 * been found.
 * Theses references can be put in events from any device.
 *
 * * Local dependencies:
 *
 * It is always possible to make 'local' references (ie: to reference an event 
 * issued by the same device without being expected).
 * An event can be localy referenced until an event (excluding HWE_ID/HWE_NULL) that does not
 * reference it is issued. I.E.: a reference to an event can be done
 * - before it is issued
 * - just after it is issued
 */

/*
 * Type of an event
 */
typedef enum hwe_type_t {
   /* HWE_ID:
    * not a real event (see below)
    * used to reset the current id to a given value when the id 
    * field has not enough range to express the new id of the next event
    */
   HWE_ID = 0,

   /*
    * HWE_NULL:
    * null/empty event (just a header),
    * should be used either as a dependencies container or to commit
    * a canceled event (as every event must be commited)
    */
   HWE_NULL = 1,

   /*
    * HWE_INFO: 
    * Information on device (see hwe_info.h)
    * Every device trace must start with one HWE_INFO event
    */
   HWE_INFO = 2,

   /*
    * HWE_SPREAD:
    * Modify number of expected references (see hwe_spread.h)
    */
   HWE_SPREAD = 3,

   /*
    * Memory accesses (see hwe_mem.h)
    */
   HWE_MEMGL = 4,     // global memory access
   HWE_MEM32,         // 32bits address memory access (must be acknowledged (or relayed) once)
   HWE_MEMACK,        // indicate that a mem access has been taken into account

   /*
    * Instruction related events (see hwe_inst.h)
    */
   HWE_INST32 = 8, // instruction on 32bits A/D architecture
   HWE_EXCEP32,    // exception in 32bits A/D architecture
   HWE_CPU_MEM,    // Request to memory
   HWE_CPU_IO,     // Request to IO

   /*
    * Event in RABBITS platform to commit events after that execution
    */
   HWE_COMMIT,
   HWE_SPARE,
} hwe_type_t;

/*
 * ID of an event
 */
typedef uint64_t hwe_id_ind_t;
#define HWE_PRI_ID PRIu64
typedef  int32_t hwe_id_ind_st;//signed version of hwe_id_ind_t
typedef uint8_t  hwe_id_dev_t;
typedef struct hwe_id_t {
   hwe_id_dev_t devid;  // device identifier
   hwe_id_ind_t index;  // index inside device events
} __attribute__((__packed__)) hwe_id_t;

static const hwe_id_t HWE_ID_NULL = { 0, 0 };

/*
 * header
 */
typedef struct hwe_head_t {
   // type
   hwe_type_t   type:4;
   // id relative to the previous event one in the trace
   int          rid:4;
   // indicate the number of dates that follow this header
   unsigned     ndates:2;
   // indicate the number of references that follow this header
   unsigned     nrefs:6;
   // indicate the number of expected followers (ie: events which refer to this one)
   unsigned     expected:32;
   //
   unsigned     nchild:32;
   unsigned     com_child:32; // :8 before flushes
} __attribute__((__packed__)) hwe_head_t;

/*
 * some defines (max elements in array of dates and refs max differntial id)
 */
#define HWE_REF_MAX 16
// #define HWE_EXP_MAX 255
#define HWE_EXP_MAX 16384 
#define HWE_DATE_MAX 2
#define HWE_RID_MAX 7
#define HWE_RID_MIN (-8)
#define HWE_CHILD_MAX 32768

/*
 * date:
 * if there is:
 *   * 2 dates -> begin "[0]" and end "[1]" date
 *   * 1 date  -> the date "[0]"
 *   * 0 date  -> ...
 * 
 */
typedef uint64_t hwe_date_t;
#define HWE_PRI_DATE PRIu64

/*
 * container
 */
typedef struct hwe_head_cont hwe_head_cont;
typedef uintptr_t hwe_ref_t;
#define HWE_REF_NULL ((hwe_ref_t) NULL)
struct hwe_head_cont {
   hwe_id_t       id;                  // whole id
   hwe_ref_t      self;
   hwe_head_t     head;                // header
   hwe_date_t     dates[HWE_DATE_MAX]; // dates
   hwe_ref_t      refs[HWE_REF_MAX];
   // ptr to next container (it's a linekd list)
   hwe_head_cont *refnext;
   // ptr to last container (if this event is the head of the list)
   hwe_head_cont *reflast;
   // Events which were generated by this one
   hwe_head_cont *child[HWE_CHILD_MAX];
   // Its generator 
   hwe_head_cont *parent;
   //Its identification on parent's child list
   uint32_t child_slot;
};


/**********************************************************************
 ********     HWE_ID     **********************************************
 **********************************************************************/

/*
 * Fake event used to set the id index to an absolute value (when relative
 * header field is not enough wide)
 *
 * Mapping:
 *    HEADER;
 *    hwe_id_ind_t id;
 */


#endif // _HWE_COMMON_H_

/**********************************************************************
 ********     TOOLS     ***********************************************
 **********************************************************************/

#ifdef HWE_USE_TOOLS
#ifndef _HWE_TOOLS_COMMON_H_
#define _HWE_TOOLS_COMMON_H_

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

static inline void hwe_head_init(hwe_head_cont *cont)
{
	cont->id = HWE_ID_NULL;
	cont->head.type = HWE_NULL;
	cont->head.ndates = 0;
	cont->head.nrefs = 0;
	cont->head.expected = 0;
    cont->head.nchild = 0;
    cont->head.com_child = 0;
    cont->child[0] = 0;
//    memset((void*)cont->child,0,sizeof(struct hwe_head_cont**)*HWE_CHILD_MAX);
	cont->refnext = NULL;
	cont->reflast = cont;
    cont->parent = NULL;
    cont->child_slot = 0;
}

static inline void hwe_head_extend(hwe_head_cont *main, hwe_head_cont *ext)
{
	main->reflast->refnext = ext;
	main->reflast = ext;
	ext->reflast = NULL;
}

/*
 * get & set a reference 
 */
static inline unsigned hwe_getnref(const hwe_head_cont *cont)
{
   return cont->head.nrefs;
}
static inline hwe_ref_t hwe_getref(const hwe_head_cont *cont, unsigned n)
{
   //n sould be < HWE_REF_MAX
   return cont->refs[n];
}
static inline void hwe_setref(hwe_head_cont *cont, unsigned n, hwe_ref_t ref)
{
   //n sould be < HWE_REF_MAX
   cont->refs[n] = ref;
}

/*
 * string for event type
 */
#define CASE(foo) case HWE_##foo : return #foo ;
static inline const char * hwe_type_getname(hwe_type_t type) {
   switch (type) {
      CASE(ID)
      CASE(NULL)
      CASE(INFO)
      CASE(SPREAD)
      CASE(MEMGL)
      CASE(MEM32)
      CASE(MEMACK)
      CASE(INST32)
      CASE(EXCEP32)
      CASE(CPU_MEM)
      CASE(CPU_IO)
      CASE(COMMIT)
      CASE(SPARE)
   }
   return NULL;
}
#undef CASE

/*
 * fill the id fields (rid + device) of the header depending on the last id index
 * if the field has not enough range:
 *    + set it to zero and return false
 *    + an HWE_ID event must be generated
 */
static inline bool hwe_head_rid_compute(hwe_head_cont *cont, hwe_id_ind_t prev_id)
{
   // rid
   hwe_id_ind_st diff = cont->id.index - prev_id;
   if (diff < HWE_RID_MIN || diff > HWE_RID_MAX) {
      cont->head.rid = 0;
      return false;
   }
   cont->head.rid = diff;
   return true;
}
/*
 * set the rid field to 0
 */
static inline void hwe_head_rid_zero(hwe_head_cont *cont)
{
   cont->head.rid = 0;
}

/*
 * set the whole id of container using header id field and previous id
 */
static inline void hwe_id_compute(hwe_head_cont *cont, hwe_id_ind_t prev_id)
{
   // index
   cont->id.index = prev_id + ((hwe_id_ind_st) cont->head.rid);
}

/*
 * compute the size needed to store an header of an event
 */
static inline size_t hwe_head_sizeof(const hwe_head_cont *cont)
{
   size_t res = sizeof(hwe_id_dev_t) + sizeof(hwe_head_t);
   res += sizeof(hwe_date_t) * cont->head.ndates;
   res += sizeof(hwe_id_t) * cont->head.nrefs;
   return res;
}

/*
 * write the header to the given memory location
 * which must have enough allocated space
 * return the address following the header
 */
static inline void * hwe_head_write(const hwe_head_cont *cont, hwe_id_t *idrefs, void *dest)
{
   size_t nbytes;

   nbytes = sizeof(hwe_id_dev_t);
   memcpy(dest, &cont->id.devid, nbytes);
   dest += nbytes;
  
   nbytes = sizeof(hwe_head_t);
   memcpy(dest, &cont->head, nbytes);
   dest += nbytes;
   
   nbytes = sizeof(hwe_date_t) * cont->head.ndates;
   memcpy(dest, &cont->dates, nbytes);
   dest += nbytes;
   
	nbytes = sizeof(hwe_id_t) * cont->head.nrefs;
	memcpy(dest, idrefs, nbytes);
	dest += nbytes;

   return dest;
}

/*
 * read
 */
static inline size_t hwe_head_read(hwe_head_cont *cont, hwe_id_t *idrefs, const void *buf, const size_t size)
{
   size_t need, cur, cur2;
   
   need = sizeof(hwe_head_t) + sizeof(hwe_id_dev_t);
   if (size < need)
      return 0;
   memcpy(&cont->id.devid, buf, sizeof(hwe_id_dev_t));
   buf += sizeof(hwe_id_dev_t);
   memcpy(&cont->head, buf, sizeof(hwe_head_t));
   buf += sizeof(hwe_head_t);

   cur = sizeof(hwe_date_t) * cont->head.ndates;
	cur2 = sizeof(hwe_id_t) * cont->head.nrefs;
   need += cur + cur2;
   if (size < need)
		return 0;
   memcpy(&cont->dates, buf, cur);
   buf += cur;
	memcpy(idrefs, buf, cur2);

   cont->id.index = 0;

   return need;
}

/*
 * print
 */
typedef hwe_id_t (*hwe_ref2id_f) (hwe_ref_t);
static inline void hwe_head_print(FILE *stream, const hwe_head_cont *cont, hwe_ref2id_f ref2id)
{
   const char *tname = hwe_type_getname(cont->head.type);
   if (tname == NULL)
      tname = "UNKNOWN";

   fprintf(stream, "HWE %u.%"HWE_PRI_ID" %s\n", cont->id.devid, cont->id.index, tname);
   if (cont->head.expected) {
      fprintf(stream, "\t expected %u reference(s)\n", cont->head.expected);
   }
   if (cont->head.ndates != 0) {
      fprintf(stream, "\t dates=["); 
      for (unsigned int i = 0; i < cont->head.ndates; i++) {
         fprintf(stream, " %"HWE_PRI_DATE, cont->dates[i]);
      }
      fprintf(stream, " ]\n");
   }
   if (cont->head.nrefs != 0) {
      fprintf(stream, "\t refs=[");
		for (const hwe_head_cont *cur = cont; cur != NULL; cur = cur->refnext) {
			for (unsigned int i = 0; i < cur->head.nrefs; i++) {
				if (ref2id == NULL) {
					fprintf(stream, " ?.?");
				} else {
					hwe_id_t id = ref2id(cur->refs[i]);
					fprintf(stream, " %u.%"HWE_PRI_ID, id.devid, id.index);
				}
			}
			if (cur->refnext != NULL)
				fprintf(stream, " |");
		}
      fprintf(stream, " ]\n");
   }
}

/*
 * desc
 */
static inline int hwe_head_desc(const hwe_head_cont *cont, hwe_ref2id_f ref2id, char *str, int len)
{
   const char *tname = hwe_type_getname(cont->head.type);
   if (tname == NULL)
      tname = "UNKNOWN";
   int cur = 0;
   if (cont->head.expected)
     cur = snprintf(str, len, "%u.%"HWE_PRI_ID"[X%u] %s", cont->id.devid, cont->id.index, 
         cont->head.expected, tname);
   else
     cur = snprintf(str, len, "%u.%"HWE_PRI_ID" %s", cont->id.devid, cont->id.index, 
         tname);
   switch (cont->head.ndates) {
      case 1:
         cur += snprintf(str + cur, len - cur, "[D%"HWE_PRI_DATE"]", cont->dates[0]);
         break;
      case 2:
         cur += snprintf(str + cur, len - cur, "[D%"HWE_PRI_DATE"+%"HWE_PRI_DATE"]", cont->dates[0], cont->dates[1] - cont->dates[0]);
         break;
      default:
         break;
   }
	unsigned nrefs = 0;
	for (const hwe_head_cont *cur = cont; cur != NULL; cur = cur->refnext)
		nrefs += cur->head.nrefs;
   switch (nrefs) {
      case 0:
         break;
      case 1:
			if (ref2id != NULL) {
				hwe_id_t id = ref2id(cont->refs[0]);
         	cur += snprintf(str+cur, len - cur, "[R %u.%"HWE_PRI_ID"]", id.devid, id.index);
			} else {
         	cur += snprintf(str+cur, len - cur, "[R ?.?]");
			}
         break;
      default:
         cur += snprintf(str+cur, len - cur, "[R #%u]", nrefs);
         break;
   }
   return cur;
}


/*
 * compute the size needed to store an header of an event
 */
static size_t HWE_ID_SIZEOF = sizeof(hwe_id_dev_t) + sizeof(hwe_head_t) + sizeof(hwe_id_ind_t);

/*
 * write the HWE_ID event corresponding to the given memory location
 * which must have enough allocated space
 * return the address following the event
 */
static inline void * hwe_id_write(const hwe_head_cont *cont, void *dest)
{
   struct {
      hwe_id_dev_t devid;
      hwe_head_t   head;
      hwe_id_ind_t index;
   } __attribute__((__packed__)) hwe_id = {
      .devid = cont->id.devid,
      .head = { 
         .type     = HWE_ID,
         .rid      = 0,
         .expected = 0,
         .nrefs    = 0,
         .ndates   = 0
      },
      .index = cont->id.index
   };
  
   size_t nbytes = sizeof(hwe_id);
   memcpy(dest, &hwe_id, nbytes);
   return dest + nbytes;
}

/*
 * read
 */
static inline size_t hwe_id_read(hwe_head_cont *cont, 
      const void *buf, const size_t size)
{
   size_t need = sizeof(hwe_id_ind_t);
   if (size < need) return 0;
   memcpy(&cont->id.index, buf, need);
   return need;
}

#endif // _HWE_TOOLS_COMMON_H_
#endif // HWE_USE_TOOLS

