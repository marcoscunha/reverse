#ifndef _HWE_MEM_H_
#define _HWE_MEM_H_

#include "hwe_common.h"

/*
 * Memory access related events
 */

/*
 * memory access type
 */
typedef enum hwe_mem_t {
   HWE_MEM_LOAD,    // load
   HWE_MEM_STORE,   // store
   HWE_MEM_MODIFY,  // modify
   HWE_MEM_WRITEBACK,// writeback
   HWE_MEM_LL,      // load linked
   HWE_MEM_SC,      // store conditional
   HWE_MEM_PREF,    // prefetch (no data)
   HWE_MEM_SYNC,    // sync (proc) (no data)
   HWE_MEM_REPL,    // replace (to caches) (no data)
   HWE_MEM_INVAL,   // invalidate (to caches) (no data)
   HWE_MEM_SW_INVAL,// invalidate (to caches) using instruction (no data)
   HWE_MEM_SW_FLUSH,// flush (to caches) using instruction (no data)
} hwe_mem_t;

/*
 * tell if use data field
 */
static inline unsigned int HWE_MEM_use_data(hwe_mem_t type) {
   switch (type) {
      case HWE_MEM_SYNC:
      case HWE_MEM_INVAL:
      case HWE_MEM_PREF:
      case HWE_MEM_REPL:
      case HWE_MEM_SW_INVAL:
      case HWE_MEM_SW_FLUSH:
         return 0;
      default:
         return 1;
   }
}


/**********************************************************************
 ********     HWE_MEM32     *******************************************
 **********************************************************************/

/*
 * Memory access on 32bits address architecture.
 * Is followed by 0 or 1 ack/relay depending on status:
 *    OK / COND -> 1
 *    FAIL      -> 0
 *
 * When referencing memory accesses and the local flag is not set, this 
 * event count as an expected reference (ie: it replace the referenced 
 * events) (note that the type should be consistent with the referenced
 * ones, ie: a LOAD event should not reference WRITE events and vice-versa).
 * For memory accesses, expected references generally represent the 'path' 
 * of an access (from issue to completiion). Whereas local references 
 * generally represents side-effets.
 * For exemple: a processor issues a LOAD, the cache will relayed the 
 * access up to the memory using expected reference but it may also invalidate
 * one of its cache line using local reference. 
 *
 * Dependencies:
 *    - Any.
 *  
 * mapping for: 
 *    HEADER;
 *    hwe_mem32_t body;
 *    uint8_t     data[body.width + 1]; // if (body.data)
 */

/*
 * Access issue on 32bits address architecture
 * Max data width: 32bits
 * should depend on:
 * - event that generates this access
 *
 * mapping for: 
 *    HEADER;
 *    hwe_mem32_t body;
 *    uint8_t     data[body.width + 1]; // if (body.data)
 */
typedef struct hwe_mem32_t {
   hwe_mem_t access:4;     // type of the access
   unsigned  inst:1;       // instruction access (ie: not data)
   unsigned  condfailed:1; // for conditional access, tell when failed
   unsigned  usedata:1;    // tell if data is present in the event
   unsigned  rcvack:1;     // tell if the request event receives the ack
   uint8_t   width;        // access width - 1 in bytes
   uint32_t  addr;         // access address
   uint8_t   cpureq_width; // width access required by cpy
   uint32_t  cpureq_addr;  // address required by cpu
} __attribute__((__packed__)) hwe_mem32_t;

typedef struct hwe_ack_t {
   hwe_mem_t access:4;     // type of the access
   uint8_t   width;        // access width - 1 in bytes
   uint32_t  addr;         // access address
  
} __attribute__((__packed__)) hwe_ack_t;


/*
 * See HWE_MEM_GLOBAL for the container.
 */


/**********************************************************************
 ********     HWE_MEMGL     *******************************************
 **********************************************************************/

/*
 * Global memory access for caches operation (mainly). There is no 
 * addr/width/data fields.
 * See HWE_MEM32 for details.
 *
 * Mapping:
 *    HEADER
 *    hwe_memglobal_t body;
 */
typedef struct hwe_memglobal_t {
   hwe_mem_t    access:4;  // type of the access
} hwe_memglobal_t;

/*
 * container
 */
#define HWE_MEM_DATA_MAX 256
typedef struct hwe_mem_cont {
   hwe_head_cont common;
   union {
      hwe_mem32_t     mem32;
      hwe_memglobal_t global;
   } body;
   uint8_t data[HWE_MEM_DATA_MAX];
} hwe_mem_cont;


/**********************************************************************
 ********     HWE_MEMACK     ******************************************
 **********************************************************************/

/*
 * Acces Acknoledgement.
 * Indicate that the an access have been made.
 *
 * The access is made given the state of the memory of the device
 * if the event has only 1 reference.
 * If there is more references than 1, then the remaining ones are also
 * taken in account for the access. Remaining references are taken in account
 * in the order of the reference, ie: the most prioritary reference is the 
 * last one.
 * 
 * The state of the memory is for example the memory array for a memory 
 * device, the cache array for a cache device, ...
 *
 * This event count as an expected reference.
 *
 * Dependencies:
 *    - At least 1.
 *    - The first reference indicate the access which is done.
 *
 * Mapping: 
 *    header;
 */
typedef struct hwe_ack_cont {
   hwe_head_cont common;
   hwe_ack_t     body;
} hwe_ack_cont;




#endif // _HWE_MEM_H_


/**********************************************************************
 ********     TOOLS     ***********************************************
 **********************************************************************/

#ifdef HWE_USE_TOOLS
#ifndef _HWE_TOOLS_MEM_H_
#define _HWE_TOOLS_MEM_H_


/*
 * string for the mem access type
 */
#define CASE(foo, str) case HWE_MEM_##foo : return str ;
static inline const char * hwe_mem_getname(hwe_mem_t type) {
   switch (type) {
      CASE(SYNC,"SYNC")
      CASE(INVAL,"INVAL")
      CASE(STORE, "STORE")
      CASE(MODIFY, "MODIFY")
      CASE(WRITEBACK, "WRITEBACK")
      CASE(LOAD, "LOAD")
      CASE(REPL, "REPLACE")
      CASE(SW_INVAL, "SOFT INVAL")
      CASE(SW_FLUSH, "SOFT FLUSH")
      CASE(PREF, "PREF")
      CASE(LL, "LL")
      CASE(SC, "SC")
   }
   return NULL;
}
#undef CASE

/*
 * sizeof
 */
static inline size_t hwe_mem32_sizeof(const hwe_mem_cont *cont)
{
   size_t res = hwe_head_sizeof(&cont->common);
   res += sizeof(hwe_mem32_t);
   if (cont->body.mem32.usedata)
      res += 1U + cont->body.mem32.width;
   return res;
}
static inline size_t hwe_memglobal_sizeof(const hwe_mem_cont *cont)
{
   return hwe_head_sizeof(&cont->common) + sizeof(hwe_memglobal_t);
}

/*
 * write the event to the given memory location
 * which must have enough allocated space
 * return the address following the event
 */
static inline void * hwe_mem32_write(const hwe_mem_cont *cont, void *dest)
{
   size_t nbytes = sizeof(hwe_mem32_t);
   memcpy(dest, &cont->body, nbytes);
   dest += nbytes;
  
   if (cont->body.mem32.usedata) {
      nbytes = 1U + cont->body.mem32.width;
      memcpy(dest, &cont->data, nbytes);
      dest += nbytes;
   }
   
   return dest;
}
static inline void * hwe_memglobal_write(const hwe_mem_cont *cont, void *dest)
{
   size_t nbytes = sizeof(hwe_memglobal_t);
   memcpy(dest, &cont->body, nbytes);
   dest += nbytes;
   
   return dest;
}

/*
 * read
 */
static inline size_t hwe_mem32_read(hwe_mem_cont *cont,
      const void *buf, const size_t size)
{
   size_t cur = sizeof(hwe_mem32_t);
   size_t need = cur;
   if (size < need) return 0;
   memcpy(&cont->body, buf, cur);
   buf += cur;

   cur = (1U + cont->body.mem32.width) * cont->body.mem32.usedata;
   need += cur;
   if (size < need) return 0;
   memcpy(&cont->data, buf, cur);

   return need;
}
static inline size_t hwe_memglobal_read(hwe_mem_cont *cont,
      const void *buf, const size_t size)
{
   size_t cur = sizeof(hwe_memglobal_t);
   size_t need = cur;
   if (size < need) return 0;
   memcpy(&cont->body, buf, cur);

   return need;
}

/*
 * print
 */
static inline void hwe_mem_print(FILE *stream, const hwe_mem_cont *cont)
{
   const char *tname = hwe_mem_getname(cont->body.global.access);
   if (tname == NULL)
      tname = "UNKNOWN";
   
   if (cont->common.head.type == HWE_MEMGL) {
      fprintf(stream, "    %s\n", tname);
   } else {
      fprintf(stream, "    %s%s%s @0x%08"PRIx32" (width:%u)\n", 
             cont->body.mem32.inst ? "I_" : "", tname,
             cont->body.mem32.condfailed ? "(failed)" : "",
             cont->body.mem32.addr, 1U + cont->body.mem32.width);
      if (cont->body.mem32.usedata) {
         fprintf(stream, "\tdata: [");
         for (unsigned int i = 0; i <= cont->body.mem32.width; i++) {
            if (i != 0 && (i % 16) == 0)
               fprintf(stream, " ,\n\t       ");
            fprintf(stream, " %02"PRIx8, cont->data[i]);
         }
         switch (cont->body.mem32.width + 1) {
            case 1:
               fprintf(stream, " ] =(le) 0x%02"PRIx8"\n", cont->data[0]);
               break;
            case 2:
               {
                  uint16_t data;
                  memcpy(&data, cont->data, 2);
                  fprintf(stream, " ] =(le) 0x%04"PRIx16"\n", data);
               }
               break;
            case 4:
               {
                  uint32_t data;
                  memcpy(&data, cont->data, 4);
                  fprintf(stream, " ] =(le) 0x%08"PRIx32"\n", data);
               }
               break;
            default:
               fprintf(stream, " ]\n");
               break;
         }
      }
   }
}

/*
 * desc
 */
static inline int hwe_mem_desc(const hwe_mem_cont *cont, char *str, int len)
{
   const char *tname = hwe_mem_getname(cont->body.global.access);
   if (tname == NULL)
      tname = "UNKNOWN";

   if (cont->common.head.type == HWE_MEMGL)
      return snprintf(str, len, "/%s", tname);
   return snprintf(str, len, "/%s%s%s 0x%08"PRIx32, 
         cont->body.mem32.inst ? "I_" : "", tname, 
         cont->body.mem32.condfailed ? "(ko)" : "", cont->body.mem32.addr);
}


/**
* @brief 
*
* @param cont
*
* @return 
*/
static inline size_t hwe_ack_sizeof(const hwe_ack_cont *cont)
{
    size_t res = hwe_head_sizeof(&cont->common);
    res += sizeof(hwe_ack_t);

    return res;
}

/**
* @brief  Write the event to the given memory location  which must have
*         enough allocated space return the address following the event
*
* @param cont
* @param dest
*
* @return 
*/
static inline void  *hwe_ack_write(const hwe_ack_cont *cont, void *dest)
{
    size_t nbytes = sizeof(hwe_ack_t);
    memcpy(dest, &cont->body, nbytes);
    dest += nbytes;

    return dest;
}

/**
* @brief 
*
* @param cont
* @param buf
* @param size
*
* @return 
*/
static inline size_t hwe_ack_read(hwe_ack_cont *cont,
                                    const void *buf, const size_t size)
{
    size_t cur = sizeof(hwe_ack_t);
    size_t need = cur;
    if (size < need) return 0;
    memcpy(&cont->body, buf, cur);
    buf += cur;

    return need;
}


/**
* @brief 
*
* @param stream
* @param cont
*/
static inline void   hwe_ack_print(FILE *stream, const hwe_ack_cont *cont)
{
    switch(cont->body.access){
    case HWE_MEM_PREF:
        fprintf(stream, "    PREF");
        break;
    case HWE_MEM_LOAD:
        fprintf(stream, "    LOAD");
        break;
    case HWE_MEM_STORE:
        fprintf(stream, "    STORE");
        break;
    default:
        fprintf(stream, "    UKNOWN");
        break;
    }
    fprintf(stream, " @0x%08"PRIx32"\n",cont->body.addr);
}

/**
* @brief 
*
* @param cont
* @param str
* @param len
*
* @return 
*/
static inline int    hwe_ack_desc(const hwe_ack_cont *cont, char *str, int len)
{
    switch(cont->body.access){
    case HWE_MEM_PREF:
        return snprintf(str, len, "PREF_ACK @ 0x%08"PRIx32, cont->body.addr);
        break;
    case HWE_MEM_LOAD:
        return snprintf(str, len, "LOAD_ACK @ 0x%08"PRIx32, cont->body.addr);
        break;
    case HWE_MEM_STORE:
        return snprintf(str, len, "STORE_ACK @ 0x%08"PRIx32, cont->body.addr);
        break;
    default:
        return snprintf(str, len, "UKNOWN @ 0x%08"PRIx32, cont->body.addr);
        break;
    }
}

#endif // _HWE_TOOLS_MEM_H_
#endif // HWE_USE_TOOLS

