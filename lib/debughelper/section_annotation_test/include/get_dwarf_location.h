#ifndef __GET_DWARF_LOCATION_H__
#define __GET_DWARF_LOCATION_H__

#include <libdebughelper.h>

#include "annotation.h"

int get_dwarf_location(debug_helper_t *dh, lock_descr_t *lock_tab, int nb_entry);

#endif /* __GET_DWARF_LOCATION_H__ */

