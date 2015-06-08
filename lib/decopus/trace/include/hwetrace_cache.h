#ifndef _HWETRACE_CACHE_H_
#define _HWETRACE_CACHE_H_

#include "hwetrace_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Setters functions for caches traces.
 *
 * defines:
 * HWE_CACHE_NODATE -> enable dates
 * HWE_CACHE_NODATA -> enable data
 */

#ifdef HWETRACE_NODATA
#define HWE_CACHE_NODATA
#endif
#ifdef HWETRACE_NODATE
#define HWE_CACHE_NODATE
#endif

/**
 *
 * @param cont event structure
 * @param ref
 */
static inline void HWE_CACHE_ack_init(hwe_cont *cont, hwe_ref_t ref)
{
#ifdef HWE_CACHE_NODATE
   HWE_HEAD_init(cont, HWE_MEMACK, 1, 0); 
#else
   HWE_HEAD_init(cont, HWE_MEMACK, 1, 1); 
#endif
   HWE_HEAD_set_ref(cont, 0, ref);
}

/**
 *
 * @param cont
 * @param date
 */
static inline void HWE_CACHE_ack_date(
      hwe_cont *cont __attribute__((__unused__)), 
      hwe_date_t date __attribute__((__unused__)))
{
#ifdef HWE_CACHE_NODATE
   ;
#else
   HWE_HEAD_set_date(cont, 0, date);
#endif
}

/**
* @brief 
*
* @param cont
* @param access
* @param addr
* @param width
*/
static inline void HWE_CACHE_ack_access(hwe_cont* cont, hwe_mem_t access ,uint32_t addr, uint32_t width)
{
    hwe_cont* head = cont;
    while(1){
        cont->mem.body.mem32.access = access;
        cont->mem.body.mem32.addr = addr;
        if (width <= HWE_MEM_DATA_MAX) {
            cont->mem.body.mem32.width = width - 1;
            break;
        }
        cont->mem.body.mem32.width = HWE_MEM_DATA_MAX - 1;
        width -= HWE_MEM_DATA_MAX;
        addr += HWE_MEM_DATA_MAX;
        if (cont->common.refnext == NULL)
            cont = hwe_extend(head);
        else
            cont = (hwe_cont *) cont->common.refnext;
        cont->common.head.type = HWE_MEMACK;
    }
}

/**
 * @brief Init event to memory initialization
 * @param cont container
 * @param nrefs
 * @param access
 * @param addr
 * @param width
 */
static inline void HWE_CACHE_mem_init(
      hwe_cont *cont,
      unsigned int nrefs,
      hwe_mem_t access,
      uint32_t addr,
      unsigned int width)
{
   HWE_MEM32_init(cont, nrefs, false, access, addr, width);
#ifdef HWE_CACHE_NODATE
   cont->common.head.ndates = 0;
#endif
#ifdef HWE_CACHE_NODATA
   cont->mem.body.mem32.usedata = 0;
#endif
}

static inline void HWE_CACHE_memgl_init(
      hwe_cont *cont,
      unsigned int nrefs,
      hwe_mem_t access)
{
   HWE_MEMGL_init(cont, nrefs, access);
#ifdef HWE_CACHE_NODATE
   cont->common.head.ndates = 0;
#endif
}

static inline void HWE_CACHE_mem_begdate(
      hwe_cont *cont __attribute__((__unused__)), 
      hwe_date_t date __attribute__((__unused__)))
{
#ifndef HWE_CACHE_NODATE
   HWE_HEAD_set_date(cont, 0, date);
#endif
}

static inline void HWE_CACHE_mem_enddate(
      hwe_cont *cont __attribute__((__unused__)), 
      hwe_date_t date __attribute__((__unused__)))
{
#ifndef HWE_CACHE_NODATE
   HWE_HEAD_set_date(cont, 1, date);
#endif
}

static inline hwe_date_t HWE_CACHE_mem_get_begdate(
      hwe_cont *cont __attribute__((__unused__)))
{
#ifndef HWE_CACHE_NODATE
   return HWE_HEAD_get_date(cont,0);
#else
   return 0;
#endif
}

static inline hwe_date_t HWE_CACHE_mem_get_enddate(
      hwe_cont *cont __attribute__((__unused__)))
{
#ifndef HWE_CACHE_NODATE
   return HWE_HEAD_get_date(cont,1);
#else
   return 0;
#endif
}

static inline void HWE_CACHE_mem_set_byte(
      hwe_cont *cont __attribute__((__unused__)),
      unsigned index __attribute__((__unused__)),
      uint8_t byte __attribute__((__unused__)))
{
#ifndef HWE_CACHE_NODATA
   HWE_MEM32_set_byte(cont, index, byte);
#endif
}

static inline void HWE_CACHE_mem_add_byte(
      hwe_cont *cont __attribute__((__unused__)),
      uint8_t byte __attribute__((__unused__)))
{
   unsigned w = HWE_MEM32_inc_width(cont, 1);
#ifndef HWE_CACHE_NODATA
   HWE_MEM32_set_byte(cont, w, byte);
#endif
}


static inline void HWE_CACHE_mem_set_word(
      hwe_cont *cont __attribute__((__unused__)),
      unsigned index __attribute__((__unused__)),
      uint32_t word __attribute__((__unused__)))
{
#ifndef HWE_CACHE_NODATA
   HWE_MEM32_set_word(cont, index, word);
#endif
}

static inline void HWE_CACHE_mem_set_data(
      hwe_cont *cont __attribute__((__unused__)), 
      void *data __attribute__((__unused__)))
{
#ifndef HWE_CACHE_NODATA
   HWE_MEM32_set_data(cont, data);
#endif
}

static inline void HWE_CACHE_mem_set_ack(
      hwe_cont *cont __attribute__((__unused__)))
{
   HWE_MEM32_set_rcvack(cont);
}

static inline unsigned HWE_CACHE_mem_get_ack(
      hwe_cont *cont __attribute__((__unused__)))
{
   return HWE_MEM32_get_rcvack(cont);
}

static inline void HWE_CACHE_set_cpureq(
      hwe_cont *cont __attribute__((__unused__)),
      uint32_t addr  __attribute__((__unused__)),
      uint32_t width __attribute__((__unused__)))
{
   HWE_MEM32_set_cpureq(cont, addr, width);
}

#ifdef __cplusplus
}
#endif

#endif // _HWETRACE_CACHE_H_

