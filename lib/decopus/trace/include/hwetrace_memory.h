#ifndef _HWETRACE_MEM_H_
#define _HWETRACE_MEM_H_

#include "hwetrace_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HWETRACE_NODATE
#define HWE_MEM_NODATE
#endif
#ifdef HWETRACE_NODATA
#define HWE_MEM_NODATA
#endif

static inline void HWE_MEM_ack_init(hwe_cont *ack)
{
#ifdef HWE_MEM_NODATE
   HWE_HEAD_init(ack, HWE_MEMACK, 1, 0);
#else
   HWE_HEAD_init(ack, HWE_MEMACK, 1, 1);
#endif
}

static inline void HWE_MEM_ack_set_date(
      hwe_cont *ack  __attribute__((__unused__)), 
      hwe_date_t     date __attribute__((__unused__)))
{
#ifdef HWE_MEM_NODATE
   ;
#else
   HWE_HEAD_set_date(ack, 0, date);
#endif
}

static inline void HWE_MEM_mem_init(
      hwe_cont *cont,
      unsigned int nrefs,
      hwe_mem_t access,
      uint32_t addr,
      unsigned int width)
{
   HWE_MEM32_init(cont, nrefs, false, access, addr, width);
#ifdef HWE_MEM_NODATE
   cont->common.head.ndates = 0;
#endif
#ifdef HWE_MEM_NODATA
   cont->mem.body.mem32.usedata = 0;
#endif
}

static inline void HWE_MEM_mem_begdate(
      hwe_cont *cont __attribute__((__unused__)), 
      hwe_date_t date __attribute__((__unused__)))
{
#ifndef HWE_MEM_NODATE
   HWE_HEAD_set_date(cont, 0, date);
#endif
}

static inline void HWE_MEM_mem_enddate(
      hwe_cont *cont __attribute__((__unused__)), 
      hwe_date_t date __attribute__((__unused__)))
{
#ifndef HWE_MEM_NODATE
   HWE_HEAD_set_date(cont, 1, date);
#endif
}

static inline void HWE_MEM_mem_set_byte(
      hwe_cont *cont __attribute__((__unused__)), 
      unsigned index __attribute__((__unused__)),
      uint8_t byte __attribute__((__unused__)))
{
#ifndef HWE_MEM_NODATA
   HWE_MEM32_set_byte(cont, index, byte);
#endif
}

static inline void HWE_MEM_mem_add_byte(
      hwe_cont *cont __attribute__((__unused__)), 
      uint8_t byte __attribute__((__unused__)))
{
   unsigned w = HWE_MEM32_inc_width(cont, 1);
#ifndef HWE_MEM_NODATA
   HWE_MEM32_set_byte(cont, w, byte);
#endif
}


static inline void HWE_MEM_mem_set_word(
      hwe_cont *cont __attribute__((__unused__)),
      unsigned index __attribute__((__unused__)),
      uint32_t word __attribute__((__unused__)))
{
#ifndef HWE_MEM_NODATA
   HWE_MEM32_set_word(cont, index, word);
#endif
}

static inline void HWE_MEM_mem_set_data(
      hwe_cont *cont __attribute__((__unused__)), 
      void *data __attribute__((__unused__)))
{
#ifndef HWE_MEM_NODATA
   HWE_MEM32_set_data(cont, data);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* _HWETRACE_MEM_H */

