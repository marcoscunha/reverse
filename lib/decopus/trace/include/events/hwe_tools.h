#ifndef _HWE_TOOLS_
#define _HWE_TOOLS_

#define HWE_USE_TOOLS

#include "hwe_events.h"


/*
 * give the size of an event
 */
#define CASE(type, name, field) \
   case HWE_##type : \
   return hwe_##name##_sizeof(&cont-> field );
static inline size_t hwe_sizeof(const hwe_cont *cont)
{
   switch ( cont->common.head.type ) {
      case HWE_ID: return HWE_ID_SIZEOF;
      CASE(NULL,head,common)
      CASE(INFO,info,info)
      CASE(SPREAD,head,common)
      CASE(MEMGL,memglobal,mem)
      CASE(MEM32,mem32,mem)
      CASE(MEMACK,ack,ack)
//      CASE(MEMACK,head,common)
      CASE(INST32,inst32,inst)
      CASE(EXCEP32,excep32,excep)
      CASE(CPU_MEM,cpu32,cpu)
      CASE(CPU_IO,cpu32,cpu)
   }
   return 0;
}
#undef CASE

/*
 * write an event to a buffer
 * it must have enough space
 */
#define CASE(type, name, field) \
   case HWE_##type : \
   return hwe_##name##_write(&cont-> field , dest);
#define CASE_EMPTY(type) \
   case HWE_##type : \
	break;
static inline void * hwe_write(const hwe_cont *cont, hwe_id_t *idrefs, void *dest)
{
	dest = hwe_head_write(&cont->common, idrefs, dest);
   switch ( cont->common.head.type ) {
      CASE(ID,id,common)
      CASE_EMPTY(NULL)
      CASE(INFO,info,info)
      CASE_EMPTY(SPREAD)
      CASE(MEMGL,memglobal,mem)
      CASE(MEM32,mem32,mem)
      CASE(MEMACK,ack,ack)
//      CASE_EMPTY(MEMACK)
      CASE(INST32,inst32,inst)
      CASE(EXCEP32,excep32,excep)
      CASE(CPU_MEM,cpu32,cpu)
      CASE(CPU_IO,cpu32,cpu)
   }
   return dest;
}
#undef CASE
#undef CASE_EMPTY

/*
 * read event from a buffer
 */
#define CASE(type, name, field) \
   case HWE_##type : \
   bodytodo = true; \
   bodysize = hwe_##name##_read(&cont-> field , buf, size); \
   break;
#define CASE_EMPTY(type) case HWE_##type : break;
static inline int hwe_read(hwe_cont *cont, hwe_id_t *idrefs, const void *buf, size_t size,
      hwe_id_ind_t *previd, unsigned pid_s) {
   size_t headsize = hwe_head_read(&cont->common, idrefs, buf, size);
   if (headsize == 0)
      return 0;

   hwe_id_compute(&cont->common, 
         (pid_s > cont->common.id.devid) ? previd[cont->common.id.devid] : 0 );

   buf += headsize;
   size -= headsize;

   size_t bodysize = 0;
   bool bodytodo = false;
   switch ( cont->common.head.type ) {
      CASE(ID, id, common)
      CASE_EMPTY(NULL)
      CASE(INFO,info,info)
      CASE_EMPTY(SPREAD)
      CASE(MEMGL,memglobal,mem)
      CASE(MEM32,mem32,mem)
      CASE(MEMACK,ack,ack)
//      CASE_EMPTY(MEMACK)
      CASE(INST32,inst32,inst)
      CASE(EXCEP32,excep32,excep)
      CASE(CPU_MEM,cpu32,cpu)
      CASE(CPU_IO,cpu32,cpu)
      default:         
         fprintf(stderr, "Unknown event => %d\n.", cont->common.head.type);
         exit(1);
         break;
   }

   if (bodytodo && bodysize == 0) {
      return 0;
   }

   return headsize + bodysize;
}
#undef CASE
#undef CASE_EMPTY

/*
 * print an event to the standard output
 */
#define CASE_EMPTY(type) case HWE_##type : break;
#define CASE(type, name, field) \
   case HWE_##type : \
   hwe_##name##_print(stream, &cont-> field ); \
   break;
static inline void hwe_print(FILE *stream, 
      const hwe_cont *cont, hwe_ref2id_f ref2id, const char *prefix, const char *suffix) {

   if (prefix)
      fprintf(stream, "%s", prefix);
   hwe_head_print(stream, &cont->common, ref2id);
	do {
		switch ( cont->common.head.type ) {
			CASE_EMPTY(ID)
			CASE_EMPTY(NULL)
			CASE(INFO,info,info)
			CASE_EMPTY(SPREAD)
			CASE(MEMGL,mem,mem)
			CASE(MEM32,mem,mem)
			CASE(MEMACK,ack,ack)
//			CASE_EMPTY(MEMACK)
			CASE(INST32,inst32,inst)
			CASE(EXCEP32,excep32,excep)
            CASE(CPU_MEM,cpu32,cpu)
            CASE(CPU_IO,cpu32,cpu)
		}
		cont = (hwe_cont *const) cont->common.refnext;
	} while (cont != NULL);
   if (suffix)
      fprintf(stream, "%s", suffix);
}
#undef CASE

/*
 * get a short description of the event in one string
 * put it in the alloated _str_ of maxlen _len_
 * return used length
 */
#define CASE(type, name, field) \
   case HWE_##type : \
   cur += hwe_##name##_desc(&cont-> field , str + cur, len - cur); \
   break;
static inline int hwe_desc(const hwe_cont *cont, hwe_ref2id_f ref2id, char *str, int len) {

   int cur = hwe_head_desc(&cont->common, ref2id, str, len);
	do {
		switch ( cont->common.head.type ) {
			CASE_EMPTY(ID)
			CASE_EMPTY(NULL)
			CASE(INFO,info,info)
			CASE_EMPTY(SPREAD)
			CASE(MEMGL,mem,mem)
			CASE(MEM32,mem,mem)
			CASE(MEMACK,ack,ack)
//			CASE_EMPTY(MEMACK)
			CASE(INST32,inst32,inst)
			CASE(EXCEP32,excep32,excep)
            CASE(CPU_MEM,cpu32,cpu)
            CASE(CPU_IO,cpu32,cpu)
		}
		cont = (hwe_cont *const) cont->common.refnext;
	} while (cont != NULL);
   return cur;
}
#undef CASE
#undef CASE_EMPTY


#endif // _HWE_TOOLS_

