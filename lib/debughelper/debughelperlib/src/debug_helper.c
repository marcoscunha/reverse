#include <stdlib.h>
#include <libdebughelper.h>

#ifdef DH_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dh"
#include <debug.h>

static void
dh_function_register(debug_helper_t *dh, function_t *func){

   ilist_elt_t *elt = NULL;
   insn_t      *insn = NULL;
   
#if 0
   if((func->declaration == 1)             ||
      (func->type == FUNCTION_TYPE_INLINE) ){
      /* Skipping declarations and inline functions for now */
      /* These are causing some difficult situations        */
      return;
   }
#else
   if(func->declaration == 1){
      /* Skipping declarations and inline functions for now */
      /* These are causing some difficult situations        */
      return;
   }

#endif

   if(func->nb_ranges != 0){
#if 1
      int i = 0;

      DMSG("Function %-30s 0x%08x %s\n",
           func->name, func->entry_pc,
           func->type == FUNCTION_TYPE_INLINE?"INL":"");

      for(i = 0; i < func->nb_ranges; i++){
         elt    = dh_search_add(dh, func->ranges[i][0]);
         insn = IL_INSN(elt);

         insn->flags |= (INSN_FLAGS_UNUSED | INSN_FLAGS_FUNC_RANGE);
         flist_add_head(&insn->func, function_copy(func));; 
         DMSG("++> range [0x%08x to 0x%08x]\n",
              func->ranges[i][0], func->ranges[i][1]);
      }
#endif
   }else{
      elt    = dh_search_add(dh, func->lowpc);
      insn = IL_INSN(elt);

      insn->flags |= (INSN_FLAGS_UNUSED | INSN_FLAGS_FUNC_ENTRY);
      flist_add_head(&insn->func, function_copy(func));; 
      //insn->func = func; 

      DMSG("Function %-30s [0x%08x to 0x%08x]%s\n",
           func->name, func->lowpc, func->highpc,
           func->type == FUNCTION_TYPE_INLINE?"INL":"");

      elt  = dh_search_add(dh, func->highpc);
      insn = elt->data;

      insn->flags |= (INSN_FLAGS_UNUSED | INSN_FLAGS_FUNC_END);
   }
   return;
}

static void
dh_src_line_register(debug_helper_t *dh){

  src_tab_t *ptab = dh->dbg->src_tabs;

  while(ptab){

    int nb_lines = ptab->nb_lines;
    int nb_files = ptab->nb_files;
    int i;
    for (i = 0; i < nb_lines; i++){

      ilist_elt_t *elt    = dh_search_add(dh, ptab->lines[i].addr);
      insn_t      *insn   = IL_INSN(elt);
      int          fileno = 0;

      /* BKS cheap way to solve a pb */
      if(!(insn->flags & INSN_FLAGS_NEW_LINE)){
        /* BKS ----------------------- */ 
        insn->flags |= (INSN_FLAGS_NEW_LINE | INSN_FLAGS_UNUSED);
        insn->line   = &(ptab->lines[i]);

        fileno       = ptab->lines[i].fileno - 1;

        if(fileno < 0 || fileno >= nb_files){
          EMSG("Trying to assign a badly indexed file : nb %d\n",
              fileno);
          insn->fname  = NULL;
        }else{
          insn->fname  = ptab->files[fileno];
        }
      }
    }
    ptab = ptab->next;
  }

  return;
}

static void
dh_fix_functions(debug_helper_t *dh){

  ilist_elt_t *pelt = dh->insn_list;

  while(pelt != NULL) {

    if( (IL_INSN(pelt)->flags & INSN_FLAGS_FUNC_ENTRY)    &&
        (IL_INSN(pelt)->func->type == FUNCTION_TYPE_FUNC) &&
        (IL_INSN(pelt)->func->highpc == 0xFFFFFFFFUL)     ){
       
       ilist_elt_t *pelt_next = pelt->next;

       while(pelt_next){
          if(IL_INSN(pelt)->func->type == FUNCTION_TYPE_FUNC)
             break;
          pelt_next = pelt_next->next;
       }

       if(pelt_next){
          IL_INSN(pelt)->func->highpc = IL_INSN(pelt->next)->addr;
          DMSG("Set end of %s to 0x%08x\n", IL_INSN(pelt)->func->name,
               IL_INSN(pelt)->func->highpc);
       }
       pelt = pelt_next;
    }else
       pelt = pelt->next;
  }
  return;
}

/*
 * Public functions
 *
 */

debug_helper_t *
dh_init(char *exec_fname){
	 
   debug_helper_t *res = malloc(sizeof(debug_helper_t));
   
   res->dbg = debug_nfo_init(exec_fname);
   res->insn_list = NULL;
   res->insn_tree = NULL;

   debug_nfo_process(res->dbg);

   flist_walk(res->dbg->funcs,
              (void(*)(void *,function_t *))dh_function_register,
              res);

   /*
    * For now we have only functions in the list/tree.
    */
   dh_fix_functions(res);

   dh_src_line_register(res);

   return res;
}

void
dh_free(debug_helper_t *dh){

   debug_nfo_closure(dh->dbg);

	/* free()-ing list and tree structures */
	itree_free(dh->insn_tree);
	ilist_free(dh->insn_list);
   
   free(dh);
}

void
dh_pretty_print(debug_helper_t *dh){
	/*                              !                            */
	IMSG("=====================================================\n");
	IMSG("|                    Dumping List                   |\n");
	IMSG("=====================================================\n");
	ilist_dump(dh->insn_list);

	IMSG("=====================================================\n");
	IMSG("|               Dumping Tree ordered                |\n");
	IMSG("=====================================================\n");
	itree_ordered_dump(dh->insn_tree);

	return;
}

ilist_elt_t *
dh_add(debug_helper_t *dh, pc_t key){

	insn_t       *insn  = insn_new(key);
	ilist_elt_t  *res   = ilist_new(insn);
	itree_node_t *node  = itree_new(key, res);
	itree_node_t *found = NULL;

	found = itree_search(dh->insn_tree, key);

	if(found){
		if(found->key == key ){
			EMSG("Add of 0x%08x impossible : already exists\n", key);
			itree_free_node(node);
			ilist_free_elt(res);
			insn_free(insn);
			return found->data;
		}
		if(found->key < key){
			/* Use the lower instruction to add */
			ilist_add(found->data, res);
			itree_insert(&(dh->insn_tree), node);
		}
		if(found->key > key){
			EMSG("Impossible situation here : itree_search gave a superior key !!!\n");
		}
	}else{ /* !found */
		/* Tree and list are empty or */
		/* all values are greater     */ 
		dh->insn_list = ilist_add(dh->insn_list, res);
		itree_insert(&(dh->insn_tree), node);
	}

	return res;
}

ilist_elt_t *
dh_search_add(debug_helper_t *dh, pc_t addr){

	ilist_elt_t  *res   = NULL;
	itree_node_t *found = NULL;

	found = itree_search(dh->insn_tree, addr);

	if(found && found->key == addr ){
		res = found->data;
	}else{ // Not Found !!!
		insn_t     *insn = insn_new(addr);
		itree_node_t *node   = NULL;

		res  = ilist_new(insn);
		node = itree_new(addr, res);

		if(found){
			if(found->key < addr){
				ilist_add(found->data, res);
				itree_insert(&(dh->insn_tree), node);
			}else{ /* found->key > key */
				EMSG("Impossible situation here : itree_search gave a superior key !!!\n");
			}
		}else{ /* !found */
			dh->insn_list = ilist_add(dh->insn_list, res);
			itree_insert(&(dh->insn_tree), node);
		}

	}
	return res;
}

ilist_elt_t *
dh_search(debug_helper_t *dh, pc_t addr){
	ilist_elt_t *res = NULL;

	itree_node_t *found = itree_search(dh->insn_tree, addr);
 
	if(found)
		res = found->data;

	return res;
}

/*
 * Do not free the returned pointer
 */
int
dh_addr2line(debug_helper_t *dh, insn_desc_t *insn, uint32_t *lineno, char **fname){

   ilist_elt_t  *curr_insn = NULL;

   uint32_t      addr   = insn->pc;
   curr_insn = itree_search_filtered_l(dh->insn_tree, addr,
                                       INSN_FLAGS_NEW_LINE);

   if(curr_insn && IL_INSN(curr_insn)->line){
      *lineno = IL_INSN(curr_insn)->line->lineno;
      *fname  = IL_INSN(curr_insn)->fname;
      return 1;
   }      
   else
      return 0;
}

get_loc_ret_t
dh_get_loc(debug_helper_t *dh, uint32_t addr, char *sym, Dwarf_Locdesc **loc,
           Dwarf_Locdesc **frame_base){

   int ret;
   function_t *funcs = NULL;
   ilist_elt_t *curr_insn = itree_search_filtered_l(dh->insn_tree, addr,
                                                    INSN_FLAGS_FUNC_ENTRY | INSN_FLAGS_FUNC_RANGE);

   DMSG("===============================================\n");
   DMSG("Looking %s @ 0x%08x\n", sym, addr);

   if(!curr_insn)
      return DH_NO_FUNC;

   funcs = IL_INSN(curr_insn)->func;
#ifdef DEBUG
   flist_dump(funcs);
#endif
   DMSG("Found in %s\n", funcs->name);
   
   ret = debug_nfo_get_loc(dh->dbg, addr, sym, loc, frame_base, funcs);

   switch(ret){
   case DBG_NFO_LOC_FOUND:
      return DH_LOC_FOUND;
   case DBG_NFO_NO_FUNC:
      return DH_NO_FUNC;
   case DBG_NFO_NO_SYM:
      return DH_NO_SYM;
   case DBG_NFO_NO_LOC:
      return DH_NO_LOC;
   }
   return DH_ERROR;
}

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
