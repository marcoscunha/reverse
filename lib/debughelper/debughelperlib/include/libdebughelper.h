#ifndef __LIBDEBUGHELPER_H__
#define __LIBDEBUGHELPER_H__

typedef struct debug_helper debug_helper_t;
typedef struct insn_desc      insn_desc_t;

#include <debug_nfo.h>
#include <insn.h>
#include <insn_list.h>
#include <insn_tree.h>
#include <functions.h>

#define INSN_FLAGS_NSEQ 0x00000001
#define INSN_FLAGS_EXC  0x00000002

struct insn_desc {
   addr_t   pc;
   uint32_t flags;
};

struct debug_helper {

   ilist_elt_t       *insn_list;
   itree_node_t      *insn_tree;
   debug_nfo_t *dbg;

};

debug_helper_t *dh_init(char *exec_fname);
void            dh_free(debug_helper_t *dh);
int             dh_addr2line(debug_helper_t *dh, insn_desc_t *insn,
                             uint32_t *lineno, char **fname);
ilist_elt_t    *dh_search(debug_helper_t *dh, pc_t addr);
ilist_elt_t    *dh_search_add(debug_helper_t *dh, pc_t addr);
ilist_elt_t    *dh_add(debug_helper_t *dh, pc_t key);
void            dh_pretty_print(debug_helper_t *dh);

typedef enum get_loc_ret get_loc_ret_t;
enum get_loc_ret {
   DH_LOC_FOUND,
   DH_NO_FUNC,
   DH_NO_SYM,
   DH_NO_LOC,
   DH_ERROR
};

get_loc_ret_t   dh_get_loc(debug_helper_t *dh, uint32_t addr, char *sym,
                           Dwarf_Locdesc  **loc, Dwarf_Locdesc **frame_base);

#endif /* __LIBDEBUGHELPER_H__ */

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
