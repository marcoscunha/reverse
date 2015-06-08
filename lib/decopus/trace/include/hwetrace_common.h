#ifndef _HWETRACE_COMMON_H_
#define _HWETRACE_COMMON_H_

#include <string.h>
#include "hwetrace.h"

#ifdef __cplusplus
extern "C" {
#endif

//for hwe_extend function
#include "hwetrace_api.h"

#ifdef HWETRACE_DEBUG
#define _HWETRACE_ATTR

void HWE_HEAD_init(hwe_cont *cont, hwe_type_t type, unsigned int nrefs, unsigned int ndates);
void HWE_HEAD_reset(hwe_cont *cont, hwe_type_t type, unsigned int nrefs,unsigned int ndates);
void HWE_HEAD_set_expected(hwe_cont *cont, unsigned int exp);
void HWE_HEAD_add_expected(hwe_cont *cont, unsigned int exp);
void HWE_HEAD_rem_expected(hwe_cont *cont, unsigned int exp);
hwe_ref_t HWE_HEAD_this(hwe_cont *cont);
void HWE_HEAD_set_date(hwe_cont *cont, unsigned index, hwe_date_t date);
void HWE_HEAD_set_nrefs(hwe_cont *cont, unsigned nrefs);
void HWE_HEAD_add_ref(hwe_cont *cont, hwe_ref_t href);
void HWE_HEAD_set_ref(hwe_cont *cont, unsigned ind, hwe_ref_t href);
void HWE_HEAD_set_child(hwe_cont* parent, hwe_cont* child);
hwe_cont* HWE_HEAD_get_child(hwe_cont*cont, int n);
void HWE_SPREAD_init(hwe_cont *cont, hwe_ref_t ref, unsigned spread);
void HWE_INFO_init(hwe_cont *cont, hwe_device_t type, const char *name, const hwe_devices_u *data);
void HWE_INFO_add(hwe_cont *cont, const hwe_devices_u *data);
void HWE_MEMGL_init(hwe_cont *cont, unsigned nrefs, hwe_mem_t access);
void HWE_MEM32_init(hwe_cont *cont, unsigned nrefs, bool  inst, hwe_mem_t access, uint32_t addr, uint32_t width);
void HWE_MEM32_set_width(hwe_cont *cont, uint32_t width);
unsigned HWE_MEM32_inc_width(hwe_cont *cont, unsigned inc);
void HWE_MEM32_set_byte(hwe_cont *cont, unsigned index, uint8_t b);
void HWE_MEM32_set_word(hwe_cont *cont, unsigned index, uint32_t w);
void HWE_MEM32_set_data(hwe_cont *cont, void *data);

#else
#define _HWETRACE_ATTR static inline
#endif

#if (!defined(HWETRACE_DEBUG)) || defined(HWETRACE_DEBUG_IMPLEM)

/**
 * @brief * header init (should be called only once)
 * @param cont
 * @param type
 * @param nrefs
 * @param ndates
 */
_HWETRACE_ATTR void HWE_HEAD_init(hwe_cont *cont,
                                 hwe_type_t type,
                                 unsigned int nrefs,
                                 unsigned int ndates)
{
   cont->common.head.type = type;
   cont->common.head.ndates = ndates;
   cont->common.head.expected = 0;
   cont->common.head.nrefs = nrefs;
   // Here, we make the hypothesis that the container has not already be
   // extended
   // and we expect _nrefs_ to be <= HWE_REF_MAX
   // use HWE_HEAD_reset instead for safe behavior in any case
}

/*
 * header reset
 * (like init but safer: handle extended events)
 */
_HWETRACE_ATTR void HWE_HEAD_reset(hwe_cont *cont,
                                  hwe_type_t type,
                                  unsigned int nrefs,
                                  unsigned int ndates)
{
   hwe_cont *head = cont;
   cont->common.head.type = type;
   cont->common.head.ndates = ndates;
   cont->common.head.expected = 0;
   while (1) {
      if (nrefs > HWE_REF_MAX) {
         cont->common.head.nrefs = HWE_REF_MAX;
         nrefs -= HWE_REF_MAX;
      } else {
         cont->common.head.nrefs = nrefs;
         nrefs = 0;
      }
      if (nrefs == 0 && cont->common.refnext == NULL)
         break;
      if (cont->common.refnext == NULL)
         cont = hwe_extend(head);
      else
         cont = (hwe_cont *) cont->common.refnext;
      cont->common.head.type = HWE_NULL;
      cont->common.head.ndates = 0;
      cont->common.head.expected = 0;
   } 
}

_HWETRACE_ATTR hwe_type_t HWE_HEAD_get_type (hwe_cont* cont)
{
  return cont->common.head.type;
}

_HWETRACE_ATTR void HWE_HEAD_set_type (hwe_cont* cont, hwe_type_t type)
{
  cont->common.head.type = type;
}

/*
 * set the number of expected followers
 */
_HWETRACE_ATTR void HWE_HEAD_set_expected(hwe_cont *cont, unsigned int exp)
{
   cont->common.head.expected = exp;
}

/**
 * @brief Increment the number of expected followers
 * @param cont
 * @param exp
 */
_HWETRACE_ATTR void HWE_HEAD_add_expected(hwe_cont *cont, unsigned int exp)
{
   cont->common.head.expected += exp;
}

/*
 * decrement the number of expected followers
 */
_HWETRACE_ATTR void HWE_HEAD_rem_expected(hwe_cont *cont, unsigned int exp)
{
   cont->common.head.expected -= exp;
}

/**
 * @brief this (ie: the thing to use to reference this event) getter
 * @param cont
 * @return
 */
_HWETRACE_ATTR hwe_ref_t HWE_HEAD_this(hwe_cont *cont)
{
   return cont->common.self;
}

/*
 * date setter
 */
_HWETRACE_ATTR void HWE_HEAD_set_date(hwe_cont *cont, 
      unsigned index, hwe_date_t date)
{
   if (index < HWE_DATE_MAX)
      cont->common.dates[index] = date;
}

_HWETRACE_ATTR hwe_date_t HWE_HEAD_get_date(hwe_cont *cont,
      unsigned index)
{
   if (index < HWE_DATE_MAX)
      return cont->common.dates[index];
   return -1;
}


/*
 * set the number of reference
 */
_HWETRACE_ATTR void HWE_HEAD_set_nrefs(hwe_cont *cont,
      unsigned nrefs)
{
   hwe_cont *head = cont;
   while (nrefs > HWE_REF_MAX) {
      nrefs -= HWE_REF_MAX;
      cont->common.head.nrefs = HWE_REF_MAX;
      if (cont->common.refnext == NULL)
         hwe_extend(head);
      cont = (hwe_cont *) cont->common.refnext;
   }
   cont->common.head.nrefs = nrefs;
}

/**
 * @brief Add a new reference
 * @param cont Source event
 * @param href Reference Event
 */

_HWETRACE_ATTR void HWE_HEAD_add_ref(hwe_cont *cont, 
      hwe_ref_t href)
{
   hwe_cont *head = cont;
   while (cont->common.head.nrefs == HWE_REF_MAX) {
      if (cont->common.refnext == NULL)
         hwe_extend(head);
      cont = (hwe_cont *) cont->common.refnext;
   }
   cont->common.refs[cont->common.head.nrefs++] = href;
}

/**
 * Set specific reference
 * @param cont
 * @param ind
 * @param href
 */
_HWETRACE_ATTR void HWE_HEAD_set_ref(hwe_cont *cont,
      unsigned ind, hwe_ref_t href)
{
   while (ind >= HWE_REF_MAX) {
      cont = (hwe_cont *) cont->common.refnext;
      if (cont == NULL)
         return;
      ind -= HWE_REF_MAX;
   }
   cont->common.refs[ind] = href;
}

/**
 * This function assumes container has just one reference
 * @param cont
 * @return
 */
_HWETRACE_ATTR hwe_ref_t HWE_HEAD_get_ref(hwe_cont *cont)
{
   return cont->common.refs[0];
}


_HWETRACE_ATTR unsigned HWE_HEAD_get_nchild(hwe_cont *cont)
{
   return cont->common.head.nchild;
}


_HWETRACE_ATTR void HWE_HEAD_set_child(hwe_cont* parent, hwe_cont* child)
{
    parent->common.child[parent->common.head.nchild] = (hwe_head_cont*)child;
    child->common.child_slot = parent->common.head.nchild;
    parent->common.head.nchild++;
    child->common.parent = (hwe_head_cont*)parent;
}


_HWETRACE_ATTR unsigned HWE_HEAD_get_com_child(hwe_cont *cont)
{
    return cont->common.head.com_child;
}


_HWETRACE_ATTR void HWE_HEAD_set_com_child(hwe_cont* cont, unsigned commited)
{
    cont->common.head.com_child = commited;
}

_HWETRACE_ATTR hwe_cont* HWE_HEAD_get_child(hwe_cont*cont, uint32_t n)
{
   return n > cont->common.head.nchild ? NULL : (hwe_cont*)cont->common.child[n];
}

_HWETRACE_ATTR void HWE_HEAD_clean_child(hwe_cont*cont)
{
   cont->common.head.nchild  = 0;
   cont->common.child[0] = NULL;
// memset(cont->common.child,0, sizeof(hwe_head_cont**)*HWE_CHILD_MAX);;
}

_HWETRACE_ATTR void HWE_HEAD_del_child(hwe_cont*cont, uint16_t child_slot)
{
   cont->common.child[child_slot] = NULL;
}

_HWETRACE_ATTR hwe_cont* HWE_HEAD_get_parent(hwe_cont* child)
{
   return (hwe_cont*)child->common.parent;
}


/*
 * init a SPREAD event
 */
_HWETRACE_ATTR void HWE_SPREAD_init(hwe_cont *cont,
      hwe_ref_t ref,
      unsigned spread)
{
   HWE_HEAD_init(cont, HWE_SPREAD, 1, 0);
   cont->common.head.expected = spread;
   HWE_HEAD_set_ref(cont, 0, ref);
}

/*
 * init an INFO event
 */
_HWETRACE_ATTR void HWE_INFO_init(
      hwe_cont *cont, 
      hwe_device_t type, 
      const char *name,
      const hwe_devices_u *data)
{
   HWE_HEAD_init(cont, HWE_INFO, 0, 0);
   cont->info.body.version = HWE_VERSION;
   cont->info.body.device = type;
   cont->info.body.nsize = 0;
   if (name != NULL) {
      unsigned i = 0;
      while (i < HWE_INFO_NAME_MAX && name[i] != '\0') {
         cont->info.name[i] = name[i];
         i += 1;
      }
      cont->info.name[i] = '\0';
      cont->info.body.nsize = i;
   }
   cont->info.detail = *data;
}

/*
 * add another INFO container
 */
_HWETRACE_ATTR void HWE_INFO_add(hwe_cont *cont, const hwe_devices_u *data)
{
   hwe_cont *head = cont;
   while (cont->common.refnext != NULL && cont->common.head.type == HWE_INFO)
      cont = (hwe_cont *) cont->common.refnext;
   if (cont->common.head.type == HWE_INFO)
      cont = hwe_extend(head);
   cont->common.head.type = HWE_INFO;
   cont->info.body.version = HWE_VERSION;
   cont->info.body.device = head->info.body.device;
   cont->info.body.nsize = 0;
   cont->info.detail = *data;
}

/*
 * init an MEMGL event
 */
_HWETRACE_ATTR void HWE_MEMGL_init(hwe_cont *cont,
      unsigned nrefs,
      hwe_mem_t access) 
{
   HWE_HEAD_init(cont, HWE_MEMGL, nrefs, 2);
   HWE_HEAD_set_expected(cont, 1);
   cont->mem.body.global.access = access;
}

/**
 * @brief Init an MEM32 event
 *
 * @param cont containner
 * @param nrefs
 * @param inst
 * @param access
 * @param addr
 * @param width
 */
_HWETRACE_ATTR void HWE_MEM32_init(hwe_cont *cont,
      unsigned nrefs,
      bool  inst,
      hwe_mem_t access, 
      uint32_t addr,
      uint32_t width)
{
   hwe_cont *head = cont;
   HWE_HEAD_init(cont, HWE_MEM32, nrefs, 2);
   HWE_HEAD_set_expected(cont, 1);
   while (1) {
      cont->mem.body.mem32.inst = inst ? 1U : 0U;
      cont->mem.body.mem32.access = access;
      cont->mem.body.mem32.condfailed = 0;
      cont->mem.body.mem32.usedata = HWE_MEM_use_data(access);
      cont->mem.body.mem32.addr = addr;
      cont->mem.body.mem32.rcvack = 0;
      cont->mem.body.mem32.cpureq_width = 0;
      cont->mem.body.mem32.cpureq_addr  = 0;
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
      cont->common.head.type = HWE_MEM32;
   }
}

/*
 * update the width of MEM32 event
 */
_HWETRACE_ATTR void HWE_MEM32_set_width(hwe_cont *cont, uint32_t width)
{
   hwe_cont *head = cont;
   uint32_t addr = head->mem.body.mem32.addr;
   while (1) {
      if (width <= HWE_MEM_DATA_MAX) {
         cont->mem.body.mem32.width = width - 1U;
         break;
      }
      cont->mem.body.mem32.width = HWE_MEM_DATA_MAX - 1U;
      addr += HWE_MEM_DATA_MAX;
      width -= HWE_MEM_DATA_MAX;
      if (cont->common.refnext == NULL)
         cont = hwe_extend(head);
      else
         cont = (hwe_cont *) cont->common.refnext;
      if (cont->common.head.type != HWE_MEM32) {
         cont->common.head.type = HWE_MEM32;
         cont->mem.body.mem32 = head->mem.body.mem32;
         cont->mem.body.mem32.addr = addr;
      }
   }
   cont = (hwe_cont *) cont->common.refnext;
   while (cont != NULL && cont->common.head.type != HWE_NULL) {
      cont->common.head.type = HWE_NULL;
      cont = (hwe_cont *) cont->common.refnext;
   }
}

/*
 * increment the width of MEM32 event
 */
_HWETRACE_ATTR unsigned HWE_MEM32_inc_width(hwe_cont *cont, unsigned inc)
{
   unsigned width = 0;
   if (1U + inc + cont->mem.body.mem32.width <= HWE_MEM_DATA_MAX) {
      width = 1U + cont->mem.body.mem32.width;
      cont->mem.body.mem32.width += inc;
   } else { 
      hwe_cont *head = cont;
      do {
         width += 1U + cont->mem.body.mem32.width;
         cont = (hwe_cont *) cont->common.refnext;
      } while (cont != NULL && cont->common.head.type == HWE_MEM32);
      HWE_MEM32_set_width(head, width + inc);
   }
   return width;
}


/*
 * set data of mem event (little endian)
 */
_HWETRACE_ATTR void HWE_MEM32_set_byte(hwe_cont *cont,
      unsigned index, uint8_t b)
{
   while (index >= HWE_MEM_DATA_MAX) {
      index -= HWE_MEM_DATA_MAX;
      cont = (hwe_cont *) cont->common.refnext;
      if (cont == NULL) //should not occur
         return;
   }
   cont->mem.data[index] = b;
}
_HWETRACE_ATTR void HWE_MEM32_set_word(hwe_cont *cont,
      unsigned index, uint32_t w)
{
   index *= 4;
   while (index >= HWE_MEM_DATA_MAX) {
      index -= HWE_MEM_DATA_MAX;
      cont = (hwe_cont *) cont->common.refnext;
      if (cont == NULL) //should not occur
         return;
   }
   uint32_t *ptr = (uint32_t *) &cont->mem.data[index];
   *ptr = w;
}
_HWETRACE_ATTR void HWE_MEM32_set_data(
      hwe_cont *cont, void *data)
{
   uint8_t *ptr = (uint8_t *) data;
   unsigned i;
   while (1) {
      for (i = 0; i <= cont->mem.body.mem32.width; i += 1)
         cont->mem.data[i] = *(ptr++);
      if (cont->mem.body.mem32.width != HWE_MEM_DATA_MAX - 1)
         break;
      cont = (hwe_cont *) cont->common.refnext;
      if (cont == NULL || cont->common.head.type != HWE_MEM32)
         break;
   }
}

_HWETRACE_ATTR void HWE_COMMIT_init(hwe_cont *cont)
{
    HWE_HEAD_init(cont,HWE_COMMIT, 0, 0);
}

_HWETRACE_ATTR void HWE_SPARE_init(hwe_cont *cont)
{
    HWE_HEAD_init(cont,HWE_SPARE, 0, 0);
    cont->mem.body.mem32.rcvack = 0;
}

static inline void HWE_MEM32_set_rcvack(
      hwe_cont *cont __attribute__((__unused__)))
{
    cont->mem.body.mem32.rcvack = 1;
}

static inline unsigned HWE_MEM32_get_rcvack(
      hwe_cont *cont __attribute__((__unused__)))
{
    return cont->mem.body.mem32.rcvack;
}

#endif
#undef _HWETRACE_ATTR

static inline void HWE_MEM32_set_cpureq(
        hwe_cont *cont __attribute__((__unused__)),
        uint32_t addr  __attribute__((__unused__)),
        uint8_t width __attribute__((__unused__)))
{
    cont->mem.body.mem32.cpureq_width = width - 1;
    cont->mem.body.mem32.cpureq_addr  = addr;
}



#ifdef __cplusplus
}
#endif

#endif // _HWETRACE_COMMON_H_

