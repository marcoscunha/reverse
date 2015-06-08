#ifndef __EXTRACT_LOCK_DESCR_H__
#define __EXTRACT_LOCK_DESCR_H__

#include <gelf.h>

#include "annotation.h"

int extract_lock_descr(Elf *e, char *section_name, lock_descr_t **lock_tab, int *nb_entry);
int extract_lock_descr_from_section(Elf_Scn *scn, lock_descr_t **lock_tab, int *nb_entry);


#endif /* __EXTRACT_LOCK_DESCR_H__ */

