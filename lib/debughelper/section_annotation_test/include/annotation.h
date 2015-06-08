#ifndef __ANNOTATION_H__
#define __ANNOTATION_H__

#include <gelf.h>
#include <libdwarf.h>
#include <dwarf.h>

/* 
 * set here the right struct contains in your sections
 */
typedef struct magic_lock_descr_ {
  unsigned long long pc;
  char name[64 - sizeof(unsigned long long)];
} magic_lock_descr_t;

typedef struct lock_descr_ {
  magic_lock_descr_t magic_lock_descr;
  Dwarf_Locdesc     *lock_location;
  Dwarf_Locdesc     *lock_frame;
} lock_descr_t;

#endif /* __ANNOTATION_H__ */

