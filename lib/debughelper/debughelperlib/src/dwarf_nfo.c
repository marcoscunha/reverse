/*
 * Copyright (c) ENS Lyon/LIP      2004-2007
 *               INSA Lyon/CITI    2004
 *               INRIA             2004
 *               INP Grenoble/Tima 2008
 *            Nicolas Fournel   <nicolas.fournel@imag.fr>
 *            Antoine Fraboulet <antoine.fraboulet@insa-lyon.fr>
 * 
 * This software is a computer program whose purpose is to process an 
 * etrace file in order to give divers informations on energy consumption.
 * 
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#include <libelf.h>
#include <libdwarf.h>
#include <dwarf.h>

#include <dwarf_nfo.h>

#ifdef DWARF_NFO_DEBUG
#define DEBUG
#endif

#define DBG_HDR "dwarf_nfo"
#include <debug.h>

#define MAX_ADDR 8


void dwarf_walk(dwarf_nfo_t *dw, function_t **funcs, Dwarf_Die die,
                unsigned long cu_idx, int no_child);


/** 
 * Error handler for Dwarf library functions.
 * 
 * @param error structure containing error description
 * @param arg opaque argument transmited from registration
 */
void
dwarf_on_error(Dwarf_Error error, __attribute__((__unused__)) Dwarf_Ptr arg){

	EMSG("libdwarf error %d : %s\n", (unsigned int)dwarf_errno(error), dwarf_errmsg(error));
	/* exit(EXIT_FAILURE); */
}

void
process_lines(Dwarf_Debug dbg, Dwarf_Die die, src_tab_t *tab){

	int res;
	Dwarf_Signed file_cnt = 0;
	Dwarf_Signed line_cnt = 0;
	Dwarf_Bool   resp     = 0;
	Dwarf_Attribute attr  = NULL;

	char       **file_buffer = NULL;
	char        *comp_dir    = NULL;
	Dwarf_Line  *line_buffer;
	int i;

	res = dwarf_hasattr(die, DW_AT_comp_dir, &resp, NULL);
	if(res != DW_DLV_OK){
		comp_dir = NULL;
	}else{
		res = dwarf_attr(die, DW_AT_comp_dir, &attr, NULL);
		if(res != DW_DLV_OK){
			comp_dir = NULL;
		}else{
			res = dwarf_formstring(attr, &comp_dir, NULL);
		}

	}

	res = dwarf_srcfiles(die, &file_buffer, &file_cnt, NULL);
	if(res != DW_DLV_OK){
		EMSG("Bad SRCFILES\n");
		file_buffer = NULL;
		file_cnt    = 0;
	}else{
		HMSG("Got %d files\n", (int)file_cnt);
    
		if(file_cnt > 0){
			tab->nb_files = file_cnt;
			tab->files = malloc(sizeof(char *)*file_cnt);
			for(i=0; i < file_cnt; i++){

				if(comp_dir && (file_buffer[i][0] != '/')) {
					tab->files[i] = malloc(strlen(file_buffer[i])+
					                       strlen(comp_dir)+2);
					strcpy(tab->files[i], comp_dir);
					strcat(tab->files[i], "/");
					strcat(tab->files[i], file_buffer[i]);
				}else{
					tab->files[i] = malloc(strlen(file_buffer[i])+1);
					strcpy(tab->files[i], file_buffer[i]);
				}
			}
      
		}

	}

	res = dwarf_srclines(die, &line_buffer, &line_cnt, NULL);
	if(res != DW_DLV_OK){
		EMSG("Bad SRCLINES !!!\n");
		line_buffer = NULL;
		line_cnt    = 0;
	}else{

		HMSG("Got %d lines\n", (int)line_cnt);

		if(line_cnt > 0){
			tab->nb_lines = line_cnt;
			tab->lines = malloc(sizeof(line_nfo_t)*line_cnt);
		}else{
			tab->nb_lines = 0;
			tab->lines    = NULL;
		}

		for(i = 0; i < line_cnt; i++){
			Dwarf_Unsigned lineno;
			Dwarf_Addr lineaddr;
			Dwarf_Unsigned fileno;
			Dwarf_Bool block_begin;
			Dwarf_Bool seq_end;

			res = dwarf_line_srcfileno(line_buffer[i], &fileno, NULL);
			if(res != DW_DLV_OK){
				EMSG("Error ... Bad SRC_FILENO!!!\n");
				fileno = 0;
			}

			res = dwarf_lineno(line_buffer[i], &lineno, NULL);
			if(res != DW_DLV_OK){
				EMSG("Error ... Bad LINENO!!!\n");
				lineno = 0;
			}

			res = dwarf_lineaddr(line_buffer[i], &lineaddr, NULL);
			if(res != DW_DLV_OK){
				EMSG("Error ... Bad LINEADDR!!!\n");
				lineaddr = 0;
			}

			res = dwarf_lineblock(line_buffer[i], &block_begin, NULL);
			if(res != DW_DLV_OK){
				EMSG("Error ... Bad LINEBLOCK!!!\n");
				block_begin = 0;
			}

			res = dwarf_lineendsequence(line_buffer[i], &seq_end, NULL);
			if(res != DW_DLV_OK){
				EMSG("Error ... Bad LINEENDSEQUENCE!!!\n");
				seq_end = 0;
			}

			tab->lines[i].addr   = lineaddr;
			tab->lines[i].fileno = fileno;
			tab->lines[i].lineno = lineno;

		}
	}
	if(line_buffer != NULL){
		for(i = 0; i < line_cnt; i++){
			dwarf_dealloc(dbg, line_buffer[i], DW_DLA_LINE);
		}
		dwarf_dealloc(dbg, line_buffer, DW_DLA_LIST);
	}
 
	if(file_buffer != NULL){
		for(i = 0; i < file_cnt; i++){
			/* printf("file : %s\n", file_buffer[i]); */
			dwarf_dealloc(dbg, file_buffer[i], DW_DLA_STRING);
		}
		dwarf_dealloc(dbg, file_buffer, DW_DLA_LIST);
	}

	return;
}


/* ---------------- */
/* Public functions */
/* ---------------- */

/** 
 * Initialization of a Dwarf2 Info structure.
 * 
 * @param elf ELF descriptor for the executable.
 * 
 * @return initialized Dwarf2 info structure 
 */
dwarf_nfo_t *
dwarf_nfo_init(Elf *elf)
{
	dwarf_nfo_t *res = malloc(sizeof(dwarf_nfo_t));
	res->dbg      = NULL;
   res->curr_func = NULL;
   
	Dwarf_Error err;
	int dw_ret;

	/* Dwarf init */
	dw_ret = dwarf_elf_init(elf, DW_DLC_READ, 
	                        dwarf_on_error, res,
	                        &(res->dbg), &err);

	if(dw_ret == DW_DLV_ERROR) {
		EMSG("Dwarf2 initialisation error\n");
		exit(EXIT_FAILURE);
	}

	HMSG("Dwarf2 Init done.\n");

	return res;
}


/** 
 * Clean of the Dwarf2 Info structure.
 * 
 * @param dw cleaned structure
 */
void
dwarf_nfo_closure(dwarf_nfo_t *dw){

	/* First close every thing */
	dwarf_finish(dw->dbg, NULL);

	/* Next freeing structures */
	free(dw);

	HMSG("Dwarf2 Closure done.\n");

	return;

}

void
dwarf_nfo_process_src(dwarf_nfo_t *dw, src_tab_t **lines)
{

	int res;
	int the_end = 0;
	unsigned long  cu_idx = 0;

	src_tab_t *tab;

	Dwarf_Unsigned cu_hdr_len;
	Dwarf_Half     ver_stamp;
	Dwarf_Unsigned abbr_off;
	Dwarf_Half     addr_size;
	Dwarf_Unsigned next_cu_hdr;
	Dwarf_Off      over_off;
	Dwarf_Off      offset;

	do{

		res = dwarf_next_cu_header(dw->dbg, &cu_hdr_len, &ver_stamp,
		                           &abbr_off, &addr_size, &next_cu_hdr, NULL);
		if(res != DW_DLV_OK) {
			if(res == DW_DLV_NO_ENTRY) {
				HMSG("No more CU\n");
				break;
			} else {
				EMSG("Error in DWARF next_cu_header()\n");
				exit(EXIT_FAILURE);
			}
			the_end = 1;
		} else {
    
			Dwarf_Die die;
			if(addr_size > MAX_ADDR) {
				EMSG("Got an unsupported address size : %d\n", addr_size);
				exit(EXIT_FAILURE);
			}

			res = dwarf_siblingof(dw->dbg, NULL, &die, NULL);
         res = dwarf_dieoffset(die, &over_off, NULL);
         res = dwarf_die_CU_offset(die, &offset, NULL);

			HMSG("<%lu><0x08%"DW_PR_DUx"> CU header len : %"DW_PR_DUu"\n",
              cu_idx, over_off - offset, cu_hdr_len);

			switch(res) {
			case DW_DLV_OK:
				/* allocate a new tab entry */
				tab = malloc(sizeof(src_tab_t));

				if(*lines != NULL) {
					tab->next = *lines;
				} else {
					tab->next = NULL;
				}
				*lines = tab;
	
				process_lines(dw->dbg, die, tab);
	
				break;
			case DW_DLV_NO_ENTRY:
				/* err_msg("Error in DWARF siblingof() : no entry\n"); */
				break;
			case DW_DLV_ERROR:
				EMSG("Error in DWARF siblingof()\n");
				exit(EXIT_FAILURE);
				break;
			default:
				EMSG("Bad return!!!\n");
			}

		}

      cu_idx++;
	}while(!the_end);

  

	return;
}

function_t *
process_subprogram_entry(dwarf_nfo_t *dw, function_t *funcs,
                         Dwarf_Die die, Dwarf_Half tag,
                         unsigned long cu_idx)
{
	Dwarf_Signed     attr_cnt;
	Dwarf_Attribute *attr_list;
	Dwarf_Off        offset;

	Dwarf_Debug dbg = dw->dbg;

	int              ret,i;
	function_t      *res;

	ret = dwarf_attrlist(die, &attr_list, &attr_cnt, NULL);
	if (ret != DW_DLV_OK) {
		return NULL;
	 } else{

		res = function_new();

		res->CU_idx = cu_idx;

		switch(tag){
		case DW_TAG_lexical_block:
			res->type = FUNCTION_TYPE_SCOPE;
			break;
		case DW_TAG_inlined_subroutine:
			res->type = FUNCTION_TYPE_INLINE;
			break;
		case DW_TAG_subroutine_type:
			res->type = FUNCTION_TYPE_FUNC;
			break;
		case DW_TAG_subprogram:
			res->type = FUNCTION_TYPE_FUNC;
			break;
		}

	}

	/*   ret = dwarf_dieoffset(die, &offset, NULL); */
	ret = dwarf_die_CU_offset(die, &offset, NULL);
	if( ret == DW_DLV_OK ){
		/* printf("Got offset : %lu\n", (unsigned long)offset); */
		res->die_off = (unsigned long)offset;
	}

	/* IMSG("Got %d attr\n", (int)attr_cnt); */
	for(i = 0; i < attr_cnt; i++){

		Dwarf_Half attr;
    
		ret = dwarf_whatattr(attr_list[i], &attr, NULL);
		if( ret == DW_DLV_OK ){

			switch(attr){
			case DW_AT_name:
			{
				char *name;
				/* IMSG("Name : \n"); */
				ret = dwarf_formstring(attr_list[i], &name, NULL);
				if (ret == DW_DLV_OK) {
					res->name = malloc(strlen(name)+1);
					DMSG("--> Name : %s\n", name);
					strcpy(res->name, name);
					dwarf_dealloc(dbg, name, DW_DLA_STRING);
				} else {
					/* Only DW_DLV_NO_ENTRY here */
					DMSG("Function at die <%lx> has name "
					     "entry with no name ...\n", (unsigned long)offset);
					/* do nothing since NULL */
				}

			}
			break;
			case DW_AT_decl_file:
			{
				/* IMSG("Decl_file : \n"); */
				Dwarf_Unsigned uval;
				/* 	  Dwarf_Half toot; */
	  
				/* 	  ret = dwarf_whatform(attr_list[i], &toot, NULL); */
				/* 	  if (ret == DW_DLV_OK) { */
				/* 	    IMSG("Got form : 0x%x\n", toot); */
				/* 	  } */
	  
				ret = dwarf_formudata(attr_list[i], &uval, NULL);
				if (ret == DW_DLV_OK) {
					res->decl_file = uval;
				} else {
					/* Only DW_DLV_NO_ENTRY here */
					DMSG("Function at die <%lx> has decl_file "
					     "entry with no value ...\n", (unsigned long)offset);
					/* do nothing since NULL */
				}
			}
			break;
			case DW_AT_decl_line:
			{
				/* IMSG("Decl_line : \n"); */
				Dwarf_Unsigned uval;
	  
				ret = dwarf_formudata(attr_list[i], &uval, NULL);
				if (ret == DW_DLV_OK) {
					res->decl_line = uval;
				} else {
					/* Only DW_DLV_NO_ENTRY here */
					DMSG("Function at die <%lx> has decl_file "
					     "entry with no value ...\n", (unsigned long)offset);
					/* do nothing since NULL */
				}
			}
			break;
			case DW_AT_low_pc:
			{
				/* IMSG("Low PC : \n"); */
				Dwarf_Addr lowpc;

				ret = dwarf_formaddr(attr_list[i], &lowpc, NULL);
				if (ret == DW_DLV_OK) {
					res->lowpc = (addr_t)lowpc;
				} else {
					/* Only DW_DLV_NO_ENTRY here */
					DMSG("Function at die <%lx> has lowpc "
					     "entry with no value\n", (unsigned long)offset);
					/* do nothing since NULL */
				}
			}
			break;
		
			case DW_AT_high_pc:
            {
                /* IMSG("High PC : \n"); */
                Dwarf_Half theform;

                ret = dwarf_whatform(attr_list[i], &theform, NULL);
                if(ret != DW_DLV_OK){
                    EMSG("Error in form retrieval\n");
                    break;
                }
                if(theform == DW_FORM_addr){
                    Dwarf_Addr highpc;

                    ret = dwarf_formaddr(attr_list[i], &highpc, NULL);
                    if (ret == DW_DLV_OK) {
                        res->highpc = (addr_t)highpc;
                    } else {
                        /* Only DW_DLV_NO_ENTRY here */
                        DMSG("Function at die <%lx> has highpc "
                             "entry with no value\n", (unsigned long)offset);
                        /* do nothing since NULL */
                        }
                }else{
                    Dwarf_Unsigned uval;
                    ret = dwarf_formudata(attr_list[i], &uval, NULL);
                    if(ret == DW_DLV_OK) {
                        if(res->lowpc == 0){
                            EMSG("Cannot set highpc ... no lowpc value\n");
                        }else{
                            res->highpc = (addr_t)(res->lowpc + uval);
                        }
                    }else{
                        DMSG("Function at die <%lx> has relative highpc "
                             "entry with no value\n", (unsigned long)offset);
                    }
                }
            }
            break;
            case DW_AT_inline:                            /* 0x20 */
            {
                Dwarf_Unsigned uval;
			    ret = dwarf_formudata(attr_list[i], &uval, NULL);

                if(ret != DW_DLV_OK){
                    EMSG("DW_AT_inline: Error in form retrieval\n");
                    break;
                }
                if (uval == DW_INL_inlined ||
                   (uval == DW_INL_declared_inlined)){
                   res->type = FUNCTION_TYPE_INLINE;
                   res->declaration = 1;
                }
		    }
            break;
            case DW_AT_abstract_origin:
            {
                /* Mostly for inlined functions ... */
                Dwarf_Off orig;

                ret = dwarf_formref(attr_list[i], &orig, NULL);
				if (ret == DW_DLV_OK) {
					res->die_origin = (unsigned long)orig;
					res->abstr_func = flist_lookup(funcs, cu_idx,
					                               (unsigned long)orig);

				if(res->abstr_func){
						DMSG("Found abstract function %s\n",
						     res->abstr_func->name);
						res->name      = res->abstr_func->name;
						res->decl_line = res->abstr_func->decl_line;
					}
				} else {
					/* Only DW_DLV_NO_ENTRY here */
					DMSG("Function at die <%lu> has abstract orig "
					     "entry with no value\n",
					     (unsigned long)offset);
					/* do nothing since NULL */
				}

			}
			break;
            case DW_AT_declaration:                       /* 0x3c */
                res->declaration = 1;
             break;

         case DW_AT_entry_pc:
         {
            Dwarf_Addr entry_pc;
            ret = dwarf_formaddr(attr_list[i], &entry_pc, NULL);
            if (ret == DW_DLV_OK) {
               DMSG("%s got an entry_pc: %x\n", res->name, (uint32_t)entry_pc);
            }
            res->entry_pc = entry_pc;
            
            break;
         }
         case DW_AT_ranges:
         {
            DMSG("%s got a ranges\n", res->name);
            Dwarf_Ranges *rangeset = 0, *prange;
            Dwarf_Signed rangecount = 0;
            Dwarf_Unsigned bytecount = 0;
            Dwarf_Addr low_pc = -1;
            Dwarf_Addr high_pc = 0;
				Dwarf_Unsigned orig;

            ret = dwarf_global_formref(attr_list[i], &orig, NULL);
            if(ret != DW_DLV_OK){
               EMSG("Cannot retrieve ranges orig\n");
               break;
            }

            ret = dwarf_get_ranges_a(dbg, orig, die, &rangeset, &rangecount,
                                     &bytecount, NULL);
            if(ret != DW_DLV_OK){
               EMSG("Cannot retrieve ranges\n");
               break;
            }

            DMSG("Got %d ranges for %s\n", (int)rangecount, res->name);

            res->ranges = malloc(sizeof(range_t)*rangecount);
            
            for(i = 0, prange = rangeset; i < rangecount; i++, prange++){

               if(prange->dwr_type == DW_RANGES_ENTRY){
                  DMSG("range[%d]: 0x%x - 0x%x\n", i,
                       (int)prange->dwr_addr1, (int)prange->dwr_addr2);

                  res->ranges[res->nb_ranges][0] = prange->dwr_addr1;
                  res->ranges[res->nb_ranges][1] = prange->dwr_addr2;
                  res->nb_ranges++;
                  
                  if(low_pc > prange->dwr_addr1)
                     low_pc = prange->dwr_addr1;
                  if(high_pc < prange->dwr_addr2)
                     high_pc = prange->dwr_addr2;
               }
            }
            res->lowpc = low_pc;
            res->highpc = high_pc;

            dwarf_ranges_dealloc(dbg,rangeset,rangecount);
            
            break;
         }

         case DW_AT_frame_base:
         {
            Dwarf_Addr base_address = dw->curr_CU_baseaddress;
            Dwarf_Half form = 0;
            ret = dwarf_whatform(attr_list[i], &form, NULL);

            if(ret != DW_DLV_OK)
               break;

				DMSG("FB : %x\n", form);
            switch(form){
            case DW_FORM_exprloc:
            {
               Dwarf_Unsigned len = 0;
               Dwarf_Ptr      data = 0;

               ret = dwarf_formexprloc(attr_list[i], &len, &data, NULL);

               if( (ret == DW_DLV_NO_ENTRY) || (ret == DW_DLV_ERROR) ){
                  EMSG("Cannot get a  DW_FORM_exprbloc....\n");
               } else {
                  Dwarf_Half address_size = 0;

                  ret = dwarf_get_die_address_size(die, &address_size, NULL);
                  if( (ret == DW_DLV_NO_ENTRY) || (ret == DW_DLV_ERROR) ){
                     EMSG("Cannot Get die address size for exprloc\n");
                  }else{
                     Dwarf_Locdesc *fb_loc = NULL;
                     Dwarf_Signed   nb_fb_locs;

                     ret = dwarf_loclist_from_expr_a(dbg, data, len,
                                                     address_size,
                                                     &fb_loc,
                                                     &nb_fb_locs, NULL);

                     if(ret == DW_DLV_OK){
                        res->fb_locs = malloc(sizeof(Dwarf_Locdesc *));
                        res->fb_locs[0] = fb_loc;
                        res->nb_fb_locs = 1;
                     }
                  }
               }
            }
            break;
            case DW_FORM_block1    : 
            case DW_FORM_block2    : 
            case DW_FORM_block4    : 
            case DW_FORM_block     :  
            case DW_FORM_data4     :  
            case DW_FORM_data8     :  
            case DW_FORM_sec_offset:
            {
               Dwarf_Locdesc **fb_loc_buf_array = NULL;
               Dwarf_Locdesc  *fb_loc = NULL;
               Dwarf_Signed nb_fb_locs;
               int iter = 0;

               ret = dwarf_loclist_n(attr_list[i], &fb_loc_buf_array, &nb_fb_locs, NULL);

               if(ret == DW_DLV_OK) {
                  res->nb_fb_locs = nb_fb_locs;
                  res->fb_locs = malloc(sizeof(Dwarf_Locdesc *)*nb_fb_locs);
               
                  for(iter = 0; iter < nb_fb_locs; iter++){

                     fb_loc = fb_loc_buf_array[iter];
                     res->fb_locs[iter] = fb_loc;

                     if(!IS_IN_FUNCTION_RANGE(fb_loc->ld_lopc, res)){ 
                        if(fb_loc->ld_lopc == 0xFFFFFFFF){
                           /*  (0xffffffff,addr), use specific address
                               (current PU address) */
                           base_address = fb_loc->ld_hipc;
                        }else{
                           /* (offset,offset), update using CU address */
                           HMSG("Updating range (%s): %x - %x : %x\n",
                                res->name, (unsigned int)fb_loc->ld_lopc,
                                (unsigned int)fb_loc->ld_hipc, (unsigned int)base_address);

                           fb_loc->ld_lopc += base_address;
                           fb_loc->ld_hipc += base_address;
                        }
                     }
                  }
                  dwarf_dealloc(dbg, fb_loc_buf_array, DW_DLA_LIST);
               }
               break;
            }
            default:
               EMSG("DW_AT_framebase: unknown form %x\n", form);
            }

            break;
         }

         /* ---------------------------------------------------------- */
			/* All remaining possible Attributes types                    */
			/* ---------------------------------------------------------- */
	
         case DW_AT_sibling                           /* 0x01 */:
         case DW_AT_location                          /* 0x02 */:
/* case DW_AT_name                              /\* 0x03 *\/: */
         case DW_AT_ordering                          /* 0x09 */:
         case DW_AT_subscr_data                       /* 0x0a */:
         case DW_AT_byte_size                         /* 0x0b */:
         case DW_AT_bit_offset                        /* 0x0c */:
         case DW_AT_bit_size                          /* 0x0d */:
         case DW_AT_element_list                      /* 0x0f */:
         case DW_AT_stmt_list                         /* 0x10 */:
/* case DW_AT_low_pc                            /\* 0x11 *\/: */
/* case DW_AT_high_pc                           /\* 0x12 *\/: */
         case DW_AT_language                          /* 0x13 */:
         case DW_AT_member                            /* 0x14 */:
         case DW_AT_discr                             /* 0x15 */:
         case DW_AT_discr_value                       /* 0x16 */:
         case DW_AT_visibility                        /* 0x17 */:
         case DW_AT_import                            /* 0x18 */:
         case DW_AT_string_length                     /* 0x19 */:
         case DW_AT_common_reference                  /* 0x1a */:
         case DW_AT_comp_dir                          /* 0x1b */:
         case DW_AT_const_value                       /* 0x1c */:
         case DW_AT_containing_type                   /* 0x1d */:
         case DW_AT_default_value                     /* 0x1e */:
/* case DW_AT_inline                            /\* 0x20 *\/: */
         case DW_AT_is_optional                       /* 0x21 */:
         case DW_AT_lower_bound                       /* 0x22 */:
         case DW_AT_producer                          /* 0x25 */:
         case DW_AT_prototyped                        /* 0x27 */:
         case DW_AT_return_addr                       /* 0x2a */:
         case DW_AT_start_scope                       /* 0x2c */:
         case DW_AT_bit_stride                        /* 0x2e */: /* DWARF3 name */
/* case DW_AT_stride_size                       /\* 0x2e *\/: /\* DWARF2 name *\/ */
         case DW_AT_upper_bound                       /* 0x2f */:
/* case DW_AT_abstract_origin                   /\* 0x31 *\/: */
         case DW_AT_accessibility                     /* 0x32 */:
         case DW_AT_address_class                     /* 0x33 */:
         case DW_AT_artificial                        /* 0x34 */:
         case DW_AT_base_types                        /* 0x35 */:
         case DW_AT_calling_convention                /* 0x36 */:
         case DW_AT_count                             /* 0x37 */:
         case DW_AT_data_member_location              /* 0x38 */:
         case DW_AT_decl_column                       /* 0x39 */:
/* case DW_AT_decl_file                         /\* 0x3a *\/: */
/* case DW_AT_decl_line                         /\* 0x3b *\/: */
/* case DW_AT_declaration                       /\* 0x3c *\/: */
         case DW_AT_discr_list                        /* 0x3d */:
         case DW_AT_encoding                          /* 0x3e */:
         case DW_AT_external                          /* 0x3f */:
         /* case DW_AT_frame_base                        /\* 0x40 *\/: */
         case DW_AT_friend                            /* 0x41 */:
         case DW_AT_identifier_case                   /* 0x42 */:
         case DW_AT_macro_info                        /* 0x43 */:
         case DW_AT_namelist_item                     /* 0x44 */:
         case DW_AT_priority                          /* 0x45 */:
         case DW_AT_segment                           /* 0x46 */:
         case DW_AT_specification                     /* 0x47 */:
         case DW_AT_static_link                       /* 0x48 */:
         case DW_AT_type                              /* 0x49 */:
         case DW_AT_use_location                      /* 0x4a */:
         case DW_AT_variable_parameter                /* 0x4b */:
         case DW_AT_virtuality                        /* 0x4c */:
         case DW_AT_vtable_elem_location              /* 0x4d */:
         case DW_AT_allocated                         /* 0x4e */: /* DWARF3 */
         case DW_AT_associated                        /* 0x4f */: /* DWARF3 */
         case DW_AT_data_location                     /* 0x50 */: /* DWARF3 */
         case DW_AT_byte_stride                       /* 0x51 */: /* DWARF3f */
/* case DW_AT_stride                            /\* 0x51 *\/: /\* DWARF3 (do not use) *\/ */
/* case DW_AT_entry_pc                          /\* 0x52 *\/: /\* DWARF3 *\/ */
         case DW_AT_use_UTF8                          /* 0x53 */: /* DWARF3 */
         case DW_AT_extension                         /* 0x54 */: /* DWARF3 */
/* case DW_AT_ranges                            /\* 0x55 *\/: /\* DWARF3 *\/ */
         case DW_AT_trampoline                        /* 0x56 */: /* DWARF3 */
         case DW_AT_call_column                       /* 0x57 */: /* DWARF3 */
         case DW_AT_call_file                         /* 0x58 */: /* DWARF3 */
         case DW_AT_call_line                         /* 0x59 */: /* DWARF3 */
         case DW_AT_description                       /* 0x5a */: /* DWARF3 */
         case DW_AT_binary_scale                      /* 0x5b */: /* DWARF3f */
         case DW_AT_decimal_scale                     /* 0x5c */: /* DWARF3f */
         case DW_AT_small                             /* 0x5d */: /* DWARF3f */
         case DW_AT_decimal_sign                      /* 0x5e */: /* DWARF3f */
         case DW_AT_digit_count                       /* 0x5f */: /* DWARF3f */
         case DW_AT_picture_string                    /* 0x60 */: /* DWARF3f */
         case DW_AT_mutable                           /* 0x61 */: /* DWARF3f */
         case DW_AT_threads_scaled                    /* 0x62 */: /* DWARF3f */
         case DW_AT_explicit                          /* 0x63 */: /* DWARF3f */
         case DW_AT_object_pointer                    /* 0x64 */: /* DWARF3f */
         case DW_AT_endianity                         /* 0x65 */: /* DWARF3f */
         case DW_AT_elemental                         /* 0x66 */: /* DWARF3f */
         case DW_AT_pure                              /* 0x67 */: /* DWARF3f */
         case DW_AT_recursive                         /* 0x68 */: /* DWARF3f */
         case DW_AT_signature                         /* 0x69 */: /* DWARF4 */
         case DW_AT_main_subprogram                   /* 0x6a */: /* DWARF4 */
         case DW_AT_data_bit_offset                   /* 0x6b */: /* DWARF4 */
         case DW_AT_const_expr                        /* 0x6c */: /* DWARF4 */
         case DW_AT_enum_class                        /* 0x6d */: /* DWARF4 */
         case DW_AT_linkage_name                      /* 0x6e */: /* DWARF4 */
#if 0
         case DW_AT_string_length_bit_size            /* 0x6f */: /* DWARF5 */
         case DW_AT_string_length_byte_size           /* 0x70 */: /* DWARF5 */
         case DW_AT_rank                              /* 0x71 */: /* DWARF5 */
         case DW_AT_str_offsets_base                  /* 0x72 */: /* DWARF5 */
         case DW_AT_addr_base                         /* 0x73 */: /* DWARF5 */
         case DW_AT_ranges_base                       /* 0x74 */: /* DWARF5 */
         case DW_AT_dwo_id                            /* 0x75 */: /* DWARF5 */
         case DW_AT_dwo_name                          /* 0x76 */: /* DWARF5 */
         case DW_AT_reference                         /* 0x77 */: /* DWARF5 */
         case DW_AT_rvalue_reference                  /* 0x78 */: /* DWARF5 */
         case DW_AT_macros                            /* 0x79 */: /* DWARF5 */
         case DW_AT_call_all_calls                    /* 0x7a */: /* DWARF5 */
         case DW_AT_call_all_source_calls             /* 0x7b */: /* DWARF5 */
         case DW_AT_call_all_tail_calls               /* 0x7c */: /* DWARF5 */
         case DW_AT_call_return_pc                    /* 0x7d */: /* DWARF5 */
         case DW_AT_call_value                        /* 0x7e */: /* DWARF5 */
         case DW_AT_call_origin                       /* 0x7f */: /* DWARF5 */
         case DW_AT_call_parameter                    /* 0x80 */: /* DWARF5 */
         case DW_AT_call_pc                           /* 0x81 */: /* DWARF5 */
         case DW_AT_call_tail_call                    /* 0x82 */: /* DWARF5 */
         case DW_AT_call_target                       /* 0x83 */: /* DWARF5 */
         case DW_AT_call_target_clobbered             /* 0x84 */: /* DWARF5 */
         case DW_AT_call_data_location                /* 0x85 */: /* DWARF5 */
         case DW_AT_call_data_value                   /* 0x86 */: /* DWARF5 */
         case DW_AT_noreturn                          /* 0x87 */: /* DWARF5 */
         case DW_AT_alignment                         /* 0x88 */: /* DWARF5 */
         case DW_AT_export_symbols                    /* 0x89 */: /* DWARF5 */
#endif

/* Follows extension so dwarfdump prints the most-likely-useful name. */
         case DW_AT_lo_user                           /* 0x2000 */:
         case DW_AT_MIPS_fde                          /* 0x2001 */: /* MIPS/SGI */
         case DW_AT_MIPS_loop_begin                   /* 0x2002 */: /* MIPS/SGI */
         case DW_AT_MIPS_tail_loop_begin              /* 0x2003 */: /* MIPS/SGI */
         case DW_AT_MIPS_epilog_begin                 /* 0x2004 */: /* MIPS/SGI */
         case DW_AT_MIPS_loop_unroll_factor           /* 0x2005 */: /* MIPS/SGI */
         case DW_AT_MIPS_software_pipeline_depth      /* 0x2006 */: /* MIPS/SGI */
         case DW_AT_MIPS_linkage_name                 /* 0x2007 */: /* MIPS/SGI, GNU, and others.*/
         case DW_AT_MIPS_stride                       /* 0x2008 */: /* MIPS/SGI */
         case DW_AT_MIPS_abstract_name                /* 0x2009 */: /* MIPS/SGI */
         case DW_AT_MIPS_clone_origin                 /* 0x200a */: /* MIPS/SGI */
         case DW_AT_MIPS_has_inlines                  /* 0x200b */: /* MIPS/SGI */
         case DW_AT_MIPS_stride_byte                  /* 0x200c */: /* MIPS/SGI */
         case DW_AT_MIPS_stride_elem                  /* 0x200d */: /* MIPS/SGI */
         case DW_AT_MIPS_ptr_dopetype                 /* 0x200e */: /* MIPS/SGI */
         case DW_AT_MIPS_allocatable_dopetype         /* 0x200f */: /* MIPS/SGI */
         case DW_AT_MIPS_assumed_shape_dopetype       /* 0x2010 */: /* MIPS/SGI */
         case DW_AT_MIPS_assumed_size                 /* 0x2011 */: /* MIPS/SGI */

            
/* GNU extensions. */
         case DW_AT_sf_names                          /* 0x2101 */: /* GNU */
         case DW_AT_src_info                          /* 0x2102 */: /* GNU */
         case DW_AT_mac_info                          /* 0x2103 */: /* GNU */
         case DW_AT_src_coords                        /* 0x2104 */: /* GNU */
         case DW_AT_body_begin                        /* 0x2105 */: /* GNU */
         case DW_AT_body_end                          /* 0x2106 */: /* GNU */
         case DW_AT_GNU_vector                        /* 0x2107 */: /* GNU */

/*  Thread safety, see http://gcc.gnu.org/wiki/ThreadSafetyAnnotation .  */
/*  The values here are from gcc-4.6.2 include/dwarf2.h.  The
    values are not given on the web page at all, nor on web pages
    it refers to. */
         case DW_AT_GNU_guarded_by                    /* 0x2108 */: /* GNU */
         case DW_AT_GNU_pt_guarded_by                 /* 0x2109 */: /* GNU */
         case DW_AT_GNU_guarded                       /* 0x210a */: /* GNU */
         case DW_AT_GNU_pt_guarded                    /* 0x210b */: /* GNU */
         case DW_AT_GNU_locks_excluded                /* 0x210c */: /* GNU */
         case DW_AT_GNU_exclusive_locks_required      /* 0x210d */: /* GNU */
         case DW_AT_GNU_shared_locks_required         /* 0x210e */: /* GNU */

/* See http://gcc.gnu.org/wiki/DwarfSeparateTypeInfo */
         case DW_AT_GNU_odr_signature                 /* 0x210f */: /* GNU */

/*  See  See http://gcc.gnu.org/wiki/TemplateParmsDwarf */
/*  The value here is from gcc-4.6.2 include/dwarf2.h.  The value is
    not consistent with the web page as of December 2011. */
         case DW_AT_GNU_template_name                 /* 0x2110 */: /* GNU */
/*  The GNU call site extension.
    See http://www.dwarfstd.org/ShowIssue.php?issue=100909.2&type=open .  */
         case DW_AT_GNU_call_site_value               /* 0x2111 */: /* GNU */
         case DW_AT_GNU_call_site_data_value          /* 0x2112 */: /* GNU */
         case DW_AT_GNU_call_site_target              /* 0x2113 */: /* GNU */
         case DW_AT_GNU_call_site_target_clobbered    /* 0x2114 */: /* GNU */
         case DW_AT_GNU_tail_call                     /* 0x2115 */: /* GNU */
         case DW_AT_GNU_all_tail_call_sites           /* 0x2116 */: /* GNU */
         case DW_AT_GNU_all_call_sites                /* 0x2117 */: /* GNU */
         case DW_AT_GNU_all_source_call_sites         /* 0x2118 */: /* GNU */
            break;
			default:
				EMSG("Unsupported attr (sub): 0x%x/%d\n", attr, attr);
			}
		}
	}

	for(i = 0; i < attr_cnt; i++) {
		dwarf_dealloc(dbg, attr_list[i], DW_DLA_ATTR);
	}
	if(attr_list == NULL) {
		printf("Got a NULL attr_list\n");
	} else {
		dwarf_dealloc(dbg, attr_list, DW_DLA_LIST);
	}

   /* +++++++++++++ */
   /* Look for vars */
   /* +++++++++++++ */
   Dwarf_Die   next_die;
   int dw_res;

   dw_res = dwarf_child(die, &next_die, NULL);
   if(dw_res == DW_DLV_OK){
      /* function_t *prev_func = dw->curr_func; */
      dw->curr_func = res;
   
      dwarf_walk(dw, NULL, next_die, cu_idx, 1);

      /* dw->curr_func = prev_func; */ /* Skipping nested functions */
      dw->curr_func = NULL;      
   }
   /* +++++++++++++ */
   /* ============= */
   /* +++++++++++++ */
   
	return res;
}

symbol_t *
process_variable_entry(dwarf_nfo_t *dw, Dwarf_Die die, unsigned long cu_idx){

	Dwarf_Debug dbg = dw->dbg;

	Dwarf_Signed     attr_cnt;
	Dwarf_Attribute *attr_list;
	Dwarf_Off        offset;
	int              ret,i;
	symbol_t        *res;

	ret = dwarf_attrlist(die, &attr_list, &attr_cnt, NULL);
	if (ret != DW_DLV_OK) {
		return NULL;
	} else {
		/* to be placed in a seperate func ? */
      res = malloc(sizeof(symbol_t));
      res->dw         = dw;
      res->name       = NULL;
      res->die_off    = 0;
      res->die_origin = 0;
      res->locs       = NULL;
      res->nb_locs    = 0;
		res->CU_idx = cu_idx;
   }

	/* ret = dwarf_dieoffset(die, &offset, NULL); */
	ret = dwarf_die_CU_offset(die, &offset, NULL);
	if(ret == DW_DLV_OK) {
		/* printf("Got offset : %lu\n", (unsigned long)offset); */
		res->die_off = (unsigned long)offset;
	}

	/* IMSG("Got %d attr\n", (int)attr_cnt); */
	for(i = 0; i < attr_cnt; i++) {

		Dwarf_Half attr;
    
		ret = dwarf_whatattr(attr_list[i], &attr, NULL);
		if(ret == DW_DLV_OK) {

			switch(attr) {
			case DW_AT_name:
			{
				char *name;
				/* IMSG("Name : \n"); */
				ret = dwarf_formstring(attr_list[i], &name, NULL);
				if (ret == DW_DLV_OK) {
					res->name = malloc(strlen(name)+1);
					strcpy(res->name, name);
				} else {
					/* Only DW_DLV_NO_ENTRY here */
					DMSG("Symbol at die <%lx> has name entry with no name\n",
					     (unsigned long)offset);
					/* do nothing since NULL */
				}
			}
			break;

			case DW_AT_abstract_origin:
			{
				Dwarf_Off orig;

				ret = dwarf_formref(attr_list[i], &orig, NULL);
				if (ret == DW_DLV_OK) {
					res->die_origin = (unsigned long)orig;
               if(dw->curr_func->abstr_func){
                  symbol_t *syms = dw->curr_func->abstr_func->syms;
                  res->abstr_sym = slist_lookup(syms, cu_idx,
                                                (unsigned long)orig);
                  if(res->abstr_sym){
                     DMSG("Found abstract sym %s\n",
                          res->abstr_sym->name);
                     res->name = malloc(strlen(res->abstr_sym->name)+1);
                     strcpy(res->name, res->abstr_sym->name);
                  }
               } else {
                  /* Only DW_DLV_NO_ENTRY here */
                  DMSG("Symbol die <%lx> has abstract orig "
                       "entry with no value ...\n", (unsigned long)offset);
                  /* do nothing since NULL */
               }
            }
			}
			break;
			case DW_AT_decl_file:
				/* IMSG("Decl_file : \n"); */
				break;
			case DW_AT_decl_line:
				/* IMSG("Decl_line : \n"); */
				break;
			case DW_AT_type:
				/* IMSG("Type : \n"); */
				break;

			case DW_AT_location:
         {

            Dwarf_Addr base_address = dw->curr_CU_baseaddress;
            Dwarf_Half form = 0;
            ret = dwarf_whatform(attr_list[i], &form, NULL);

            if(ret != DW_DLV_OK)
               break;

				DMSG("Loc : %x\n", form);

            switch(form){
            case DW_FORM_exprloc:
            {
               Dwarf_Unsigned len = 0;
               Dwarf_Ptr      data = 0;

               ret = dwarf_formexprloc(attr_list[i], &len, &data, NULL);

               if( (ret == DW_DLV_NO_ENTRY) || (ret == DW_DLV_ERROR) ){
                  EMSG("Cannot get a  DW_FORM_exprbloc....\n");
               } else {
                  Dwarf_Half address_size = 0;

                  ret = dwarf_get_die_address_size(die, &address_size, NULL);
                  if( (ret == DW_DLV_NO_ENTRY) || (ret == DW_DLV_ERROR) ){
                     EMSG("Cannot Get die address size for exprloc\n");
                  }else{
                     Dwarf_Locdesc *loc = NULL;
                     Dwarf_Signed   nb_locs;

                     ret = dwarf_loclist_from_expr_a(dbg, data, len,
                                                     address_size,
                                                     &loc,
                                                     &nb_locs, NULL);

                     if(ret == DW_DLV_OK){
                        res->locs = malloc(sizeof(Dwarf_Locdesc *));
                        res->locs[0] = loc;
                        res->nb_locs = 1;
                     }
                  }
               }
            }
            break;
            case DW_FORM_block1    : 
            case DW_FORM_block2    : 
            case DW_FORM_block4    : 
            case DW_FORM_block     :  
            case DW_FORM_data4     :  
            case DW_FORM_data8     :  
            case DW_FORM_sec_offset:
            {
               Dwarf_Locdesc **loc_buf_array = NULL;
               Dwarf_Locdesc  *loc = NULL;
               Dwarf_Signed nb_locs;
               int iter = 0;

               ret = dwarf_loclist_n(attr_list[i], &loc_buf_array, &nb_locs, NULL);

               if(ret == DW_DLV_OK) {
                  res->nb_locs = nb_locs;
                  res->locs = malloc(sizeof(Dwarf_Locdesc *)*nb_locs);
               
                  for(iter = 0; iter < nb_locs; iter++){

                     loc = loc_buf_array[iter];
                     res->locs[iter] = loc;

                     if(!IS_IN_FUNCTION_RANGE(loc->ld_lopc, dw->curr_func)){ 
                        if(loc->ld_lopc == 0xFFFFFFFF){
                           /*  (0xffffffff,addr), use specific address
                               (current PU address) */
                           base_address = loc->ld_hipc;
                        }else{
                           /* (offset,offset), update using CU address */
                           HMSG("Updating range (%s): %x - %x : %x\n",
                                res->name, (unsigned int)loc->ld_lopc,
                                (unsigned int)loc->ld_hipc, (unsigned int)base_address);

                           loc->ld_lopc += base_address;
                           loc->ld_hipc += base_address;
                        }
                     }
                  }
                  dwarf_dealloc(dbg, loc_buf_array, DW_DLA_LIST);
               }
               break;
            }
            default:
               EMSG("DW_AT_location: unknown form %x\n", form);
            }
         }
         break;

			case DW_AT_byte_size:
				/* for variable size */
				break;

         
         case DW_AT_sibling                           /* 0x01 */:
/* case DW_AT_location                          /\* 0x02 *\/: */
/* case DW_AT_name                              /\* 0x03 *\/: */
         case DW_AT_ordering                          /* 0x09 */:
         case DW_AT_subscr_data                       /* 0x0a */:
/* case DW_AT_byte_size                         /\* 0x0b *\/: */
         case DW_AT_bit_offset                        /* 0x0c */:
         case DW_AT_bit_size                          /* 0x0d */:
         case DW_AT_element_list                      /* 0x0f */:
         case DW_AT_stmt_list                         /* 0x10 */:
         case DW_AT_low_pc                            /* 0x11 */:
         case DW_AT_high_pc                           /* 0x12 */:
         case DW_AT_language                          /* 0x13 */:
         case DW_AT_member                            /* 0x14 */:
         case DW_AT_discr                             /* 0x15 */:
         case DW_AT_discr_value                       /* 0x16 */:
         case DW_AT_visibility                        /* 0x17 */:
         case DW_AT_import                            /* 0x18 */:
         case DW_AT_string_length                     /* 0x19 */:
         case DW_AT_common_reference                  /* 0x1a */:
         case DW_AT_comp_dir                          /* 0x1b */:
         case DW_AT_const_value                       /* 0x1c */:
         case DW_AT_containing_type                   /* 0x1d */:
         case DW_AT_default_value                     /* 0x1e */:
         case DW_AT_inline                            /* 0x20 */:
         case DW_AT_is_optional                       /* 0x21 */:
         case DW_AT_lower_bound                       /* 0x22 */:
         case DW_AT_producer                          /* 0x25 */:
         case DW_AT_prototyped                        /* 0x27 */:
         case DW_AT_return_addr                       /* 0x2a */:
         case DW_AT_start_scope                       /* 0x2c */:
         case DW_AT_bit_stride                        /* 0x2e */: /* DWARF3 name */
         /* case DW_AT_stride_size                       /\* 0x2e *\/: /\* DWARF2 name *\/ */
         case DW_AT_upper_bound                       /* 0x2f */:
/* case DW_AT_abstract_origin                   /\* 0x31 *\/: */
         case DW_AT_accessibility                     /* 0x32 */:
         case DW_AT_address_class                     /* 0x33 */:
         case DW_AT_artificial                        /* 0x34 */:
         case DW_AT_base_types                        /* 0x35 */:
         case DW_AT_calling_convention                /* 0x36 */:
         case DW_AT_count                             /* 0x37 */:
         case DW_AT_data_member_location              /* 0x38 */:
         case DW_AT_decl_column                       /* 0x39 */:
/* case DW_AT_decl_file                         /\* 0x3a *\/: */
/* case DW_AT_decl_line                         /\* 0x3b *\/: */
         case DW_AT_declaration                       /* 0x3c */:
         case DW_AT_discr_list                        /* 0x3d */:
         case DW_AT_encoding                          /* 0x3e */:
         case DW_AT_external                          /* 0x3f */:
         case DW_AT_frame_base                        /* 0x40 */:
         case DW_AT_friend                            /* 0x41 */:
         case DW_AT_identifier_case                   /* 0x42 */:
         case DW_AT_macro_info                        /* 0x43 */:
         case DW_AT_namelist_item                     /* 0x44 */:
         case DW_AT_priority                          /* 0x45 */:
         case DW_AT_segment                           /* 0x46 */:
         case DW_AT_specification                     /* 0x47 */:
         case DW_AT_static_link                       /* 0x48 */:
/* case DW_AT_type                              /\* 0x49 *\/: */
         case DW_AT_use_location                      /* 0x4a */:
         case DW_AT_variable_parameter                /* 0x4b */:
         case DW_AT_virtuality                        /* 0x4c */:
         case DW_AT_vtable_elem_location              /* 0x4d */:
         case DW_AT_allocated                         /* 0x4e */: /* DWARF3 */
         case DW_AT_associated                        /* 0x4f */: /* DWARF3 */
         case DW_AT_data_location                     /* 0x50 */: /* DWARF3 */
         case DW_AT_byte_stride                       /* 0x51 */: /* DWARF3f */
         /* case DW_AT_stride                            /\* 0x51 *\/: /\* DWARF3 (do not use) *\/ */
         case DW_AT_entry_pc                          /* 0x52 */: /* DWARF3 */
         case DW_AT_use_UTF8                          /* 0x53 */: /* DWARF3 */
         case DW_AT_extension                         /* 0x54 */: /* DWARF3 */
         case DW_AT_ranges                            /* 0x55 */: /* DWARF3 */
         case DW_AT_trampoline                        /* 0x56 */: /* DWARF3 */
         case DW_AT_call_column                       /* 0x57 */: /* DWARF3 */
         case DW_AT_call_file                         /* 0x58 */: /* DWARF3 */
         case DW_AT_call_line                         /* 0x59 */: /* DWARF3 */
         case DW_AT_description                       /* 0x5a */: /* DWARF3 */
         case DW_AT_binary_scale                      /* 0x5b */: /* DWARF3f */
         case DW_AT_decimal_scale                     /* 0x5c */: /* DWARF3f */
         case DW_AT_small                             /* 0x5d */: /* DWARF3f */
         case DW_AT_decimal_sign                      /* 0x5e */: /* DWARF3f */
         case DW_AT_digit_count                       /* 0x5f */: /* DWARF3f */
         case DW_AT_picture_string                    /* 0x60 */: /* DWARF3f */
         case DW_AT_mutable                           /* 0x61 */: /* DWARF3f */
         case DW_AT_threads_scaled                    /* 0x62 */: /* DWARF3f */
         case DW_AT_explicit                          /* 0x63 */: /* DWARF3f */
         case DW_AT_object_pointer                    /* 0x64 */: /* DWARF3f */
         case DW_AT_endianity                         /* 0x65 */: /* DWARF3f */
         case DW_AT_elemental                         /* 0x66 */: /* DWARF3f */
         case DW_AT_pure                              /* 0x67 */: /* DWARF3f */
         case DW_AT_recursive                         /* 0x68 */: /* DWARF3f */
         case DW_AT_signature                         /* 0x69 */: /* DWARF4 */
         case DW_AT_main_subprogram                   /* 0x6a */: /* DWARF4 */
         case DW_AT_data_bit_offset                   /* 0x6b */: /* DWARF4 */
         case DW_AT_const_expr                        /* 0x6c */: /* DWARF4 */
         case DW_AT_enum_class                        /* 0x6d */: /* DWARF4 */
         case DW_AT_linkage_name                      /* 0x6e */: /* DWARF4 */
#if 0
         case DW_AT_string_length_bit_size            /* 0x6f */: /* DWARF5 */
         case DW_AT_string_length_byte_size           /* 0x70 */: /* DWARF5 */
         case DW_AT_rank                              /* 0x71 */: /* DWARF5 */
         case DW_AT_str_offsets_base                  /* 0x72 */: /* DWARF5 */
         case DW_AT_addr_base                         /* 0x73 */: /* DWARF5 */
         case DW_AT_ranges_base                       /* 0x74 */: /* DWARF5 */
         case DW_AT_dwo_id                            /* 0x75 */: /* DWARF5 */
         case DW_AT_dwo_name                          /* 0x76 */: /* DWARF5 */
         case DW_AT_reference                         /* 0x77 */: /* DWARF5 */
         case DW_AT_rvalue_reference                  /* 0x78 */: /* DWARF5 */
         case DW_AT_macros                            /* 0x79 */: /* DWARF5 */
         case DW_AT_call_all_calls                    /* 0x7a */: /* DWARF5 */
         case DW_AT_call_all_source_calls             /* 0x7b */: /* DWARF5 */
         case DW_AT_call_all_tail_calls               /* 0x7c */: /* DWARF5 */
         case DW_AT_call_return_pc                    /* 0x7d */: /* DWARF5 */
         case DW_AT_call_value                        /* 0x7e */: /* DWARF5 */
         case DW_AT_call_origin                       /* 0x7f */: /* DWARF5 */
         case DW_AT_call_parameter                    /* 0x80 */: /* DWARF5 */
         case DW_AT_call_pc                           /* 0x81 */: /* DWARF5 */
         case DW_AT_call_tail_call                    /* 0x82 */: /* DWARF5 */
         case DW_AT_call_target                       /* 0x83 */: /* DWARF5 */
         case DW_AT_call_target_clobbered             /* 0x84 */: /* DWARF5 */
         case DW_AT_call_data_location                /* 0x85 */: /* DWARF5 */
         case DW_AT_call_data_value                   /* 0x86 */: /* DWARF5 */
         case DW_AT_noreturn                          /* 0x87 */: /* DWARF5 */
         case DW_AT_alignment                         /* 0x88 */: /* DWARF5 */
         case DW_AT_export_symbols                    /* 0x89 */: /* DWARF5 */
#endif

/* GNU extensions. */
         case DW_AT_sf_names                          /* 0x2101 */: /* GNU */
         case DW_AT_src_info                          /* 0x2102 */: /* GNU */
         case DW_AT_mac_info                          /* 0x2103 */: /* GNU */
         case DW_AT_src_coords                        /* 0x2104 */: /* GNU */
         case DW_AT_body_begin                        /* 0x2105 */: /* GNU */
         case DW_AT_body_end                          /* 0x2106 */: /* GNU */
         case DW_AT_GNU_vector                        /* 0x2107 */: /* GNU */

/*  Thread safety, see http://gcc.gnu.org/wiki/ThreadSafetyAnnotation .  */
/*  The values here are from gcc-4.6.2 include/dwarf2.h.  The
    values are not given on the web page at all, nor on web pages
    it refers to. */
         case DW_AT_GNU_guarded_by                    /* 0x2108 */: /* GNU */
         case DW_AT_GNU_pt_guarded_by                 /* 0x2109 */: /* GNU */
         case DW_AT_GNU_guarded                       /* 0x210a */: /* GNU */
         case DW_AT_GNU_pt_guarded                    /* 0x210b */: /* GNU */
         case DW_AT_GNU_locks_excluded                /* 0x210c */: /* GNU */
         case DW_AT_GNU_exclusive_locks_required      /* 0x210d */: /* GNU */
         case DW_AT_GNU_shared_locks_required         /* 0x210e */: /* GNU */

/* See http://gcc.gnu.org/wiki/DwarfSeparateTypeInfo */
         case DW_AT_GNU_odr_signature                 /* 0x210f */: /* GNU */

/*  See  See http://gcc.gnu.org/wiki/TemplateParmsDwarf */
/*  The value here is from gcc-4.6.2 include/dwarf2.h.  The value is
    not consistent with the web page as of December 2011. */
         case DW_AT_GNU_template_name                 /* 0x2110 */: /* GNU */
/*  The GNU call site extension.
    See http://www.dwarfstd.org/ShowIssue.php?issue=100909.2&type=open .  */
         case DW_AT_GNU_call_site_value               /* 0x2111 */: /* GNU */
         case DW_AT_GNU_call_site_data_value          /* 0x2112 */: /* GNU */
         case DW_AT_GNU_call_site_target              /* 0x2113 */: /* GNU */
         case DW_AT_GNU_call_site_target_clobbered    /* 0x2114 */: /* GNU */
         case DW_AT_GNU_tail_call                     /* 0x2115 */: /* GNU */
         case DW_AT_GNU_all_tail_call_sites           /* 0x2116 */: /* GNU */
         case DW_AT_GNU_all_call_sites                /* 0x2117 */: /* GNU */
         case DW_AT_GNU_all_source_call_sites         /* 0x2118 */: /* GNU */
            break;

			default:
				EMSG("Unsupported attr (var): 0x%x/%d\n", attr, attr);
			}
		}

		dwarf_dealloc(dbg, attr_list[i], DW_DLA_ATTR);
	}
	dwarf_dealloc(dbg, attr_list, DW_DLA_LIST);

	return res;
}


void
process_compile_unit(dwarf_nfo_t *dw, Dwarf_Die die, unsigned long cu_idx){

   (void)cu_idx;
   Dwarf_Debug dbg = dw->dbg;

	Dwarf_Signed     attr_cnt;
	Dwarf_Attribute *attr_list;
   int              ret, i;
   
	ret = dwarf_attrlist(die, &attr_list, &attr_cnt, NULL);
	if (ret != DW_DLV_OK) {
		return;
   }

	for(i = 0; i < attr_cnt; i++) {

		Dwarf_Half attr;
    
		ret = dwarf_whatattr(attr_list[i], &attr, NULL);
		if(ret == DW_DLV_OK) {
         switch(attr) {
         case DW_AT_low_pc                            /* 0x11 */:
         {
            Dwarf_Addr baseaddress = 0;
            ret = dwarf_formaddr(attr_list[i], &baseaddress, NULL);
            if(ret == DW_DLV_OK){
               dw->curr_CU_baseaddress = baseaddress;
            }else{
               EMSG("Bad CU baseaddress\n");
            }
         }  
         break;
         case DW_AT_high_pc                           /* 0x12 */:
         {
            Dwarf_Addr topaddress = 0;
            ret = dwarf_formaddr(attr_list[i], &topaddress, NULL);
            if(ret == DW_DLV_OK){
               dw->curr_CU_topaddress = topaddress;
            }else{
               EMSG("Bad CU topaddress\n");
            }
         }
         break;
            
         case DW_AT_sibling                           /* 0x01 */:
         case DW_AT_location                          /* 0x02 */:
         case DW_AT_name                              /* 0x03 */:
         case DW_AT_ordering                          /* 0x09 */:
         case DW_AT_subscr_data                       /* 0x0a */:
         case DW_AT_byte_size                         /* 0x0b */:
         case DW_AT_bit_offset                        /* 0x0c */:
         case DW_AT_bit_size                          /* 0x0d */:
         case DW_AT_element_list                      /* 0x0f */:
         case DW_AT_stmt_list                         /* 0x10 */:
         /* case DW_AT_low_pc                            /\* 0x11 *\/: */
         /* case DW_AT_high_pc                           /\* 0x12 *\/: */
         case DW_AT_language                          /* 0x13 */:
         case DW_AT_member                            /* 0x14 */:
         case DW_AT_discr                             /* 0x15 */:
         case DW_AT_discr_value                       /* 0x16 */:
         case DW_AT_visibility                        /* 0x17 */:
         case DW_AT_import                            /* 0x18 */:
         case DW_AT_string_length                     /* 0x19 */:
         case DW_AT_common_reference                  /* 0x1a */:
         case DW_AT_comp_dir                          /* 0x1b */:
         case DW_AT_const_value                       /* 0x1c */:
         case DW_AT_containing_type                   /* 0x1d */:
         case DW_AT_default_value                     /* 0x1e */:
         case DW_AT_inline                            /* 0x20 */:
         case DW_AT_is_optional                       /* 0x21 */:
         case DW_AT_lower_bound                       /* 0x22 */:
         case DW_AT_producer                          /* 0x25 */:
         case DW_AT_prototyped                        /* 0x27 */:
         case DW_AT_return_addr                       /* 0x2a */:
         case DW_AT_start_scope                       /* 0x2c */:
         /* case DW_AT_bit_stride                        /\* 0x2e *\/: /\* DWARF3 name *\/ */
         case DW_AT_stride_size                       /* 0x2e */: /* DWARF2 name */
         case DW_AT_upper_bound                       /* 0x2f */:
         case DW_AT_abstract_origin                   /* 0x31 */:
         case DW_AT_accessibility                     /* 0x32 */:
         case DW_AT_address_class                     /* 0x33 */:
         case DW_AT_artificial                        /* 0x34 */:
         case DW_AT_base_types                        /* 0x35 */:
         case DW_AT_calling_convention                /* 0x36 */:
         case DW_AT_count                             /* 0x37 */:
         case DW_AT_data_member_location              /* 0x38 */:
         case DW_AT_decl_column                       /* 0x39 */:
/* case DW_AT_decl_file                         /\* 0x3a *\/: */
/* case DW_AT_decl_line                         /\* 0x3b *\/: */
         case DW_AT_declaration                       /* 0x3c */:
         case DW_AT_discr_list                        /* 0x3d */:
         case DW_AT_encoding                          /* 0x3e */:
         case DW_AT_external                          /* 0x3f */:
         case DW_AT_frame_base                        /* 0x40 */:
         case DW_AT_friend                            /* 0x41 */:
         case DW_AT_identifier_case                   /* 0x42 */:
         case DW_AT_macro_info                        /* 0x43 */:
         case DW_AT_namelist_item                     /* 0x44 */:
         case DW_AT_priority                          /* 0x45 */:
         case DW_AT_segment                           /* 0x46 */:
         case DW_AT_specification                     /* 0x47 */:
         case DW_AT_static_link                       /* 0x48 */:
/* case DW_AT_type                              /\* 0x49 *\/: */
         case DW_AT_use_location                      /* 0x4a */:
         case DW_AT_variable_parameter                /* 0x4b */:
         case DW_AT_virtuality                        /* 0x4c */:
         case DW_AT_vtable_elem_location              /* 0x4d */:
         case DW_AT_allocated                         /* 0x4e */: /* DWARF3 */
         case DW_AT_associated                        /* 0x4f */: /* DWARF3 */
         case DW_AT_data_location                     /* 0x50 */: /* DWARF3 */
         case DW_AT_byte_stride                       /* 0x51 */: /* DWARF3f */
         /* case DW_AT_stride                            /\* 0x51 *\/: /\* DWARF3 (do not use) *\/ */
         case DW_AT_entry_pc                          /* 0x52 */: /* DWARF3 */
         case DW_AT_use_UTF8                          /* 0x53 */: /* DWARF3 */
         case DW_AT_extension                         /* 0x54 */: /* DWARF3 */
         case DW_AT_ranges                            /* 0x55 */: /* DWARF3 */
         case DW_AT_trampoline                        /* 0x56 */: /* DWARF3 */
         case DW_AT_call_column                       /* 0x57 */: /* DWARF3 */
         case DW_AT_call_file                         /* 0x58 */: /* DWARF3 */
         case DW_AT_call_line                         /* 0x59 */: /* DWARF3 */
         case DW_AT_description                       /* 0x5a */: /* DWARF3 */
         case DW_AT_binary_scale                      /* 0x5b */: /* DWARF3f */
         case DW_AT_decimal_scale                     /* 0x5c */: /* DWARF3f */
         case DW_AT_small                             /* 0x5d */: /* DWARF3f */
         case DW_AT_decimal_sign                      /* 0x5e */: /* DWARF3f */
         case DW_AT_digit_count                       /* 0x5f */: /* DWARF3f */
         case DW_AT_picture_string                    /* 0x60 */: /* DWARF3f */
         case DW_AT_mutable                           /* 0x61 */: /* DWARF3f */
         case DW_AT_threads_scaled                    /* 0x62 */: /* DWARF3f */
         case DW_AT_explicit                          /* 0x63 */: /* DWARF3f */
         case DW_AT_object_pointer                    /* 0x64 */: /* DWARF3f */
         case DW_AT_endianity                         /* 0x65 */: /* DWARF3f */
         case DW_AT_elemental                         /* 0x66 */: /* DWARF3f */
         case DW_AT_pure                              /* 0x67 */: /* DWARF3f */
         case DW_AT_recursive                         /* 0x68 */: /* DWARF3f */
         case DW_AT_signature                         /* 0x69 */: /* DWARF4 */
         case DW_AT_main_subprogram                   /* 0x6a */: /* DWARF4 */
         case DW_AT_data_bit_offset                   /* 0x6b */: /* DWARF4 */
         case DW_AT_const_expr                        /* 0x6c */: /* DWARF4 */
         case DW_AT_enum_class                        /* 0x6d */: /* DWARF4 */
         case DW_AT_linkage_name                      /* 0x6e */: /* DWARF4 */
#if 0
         case DW_AT_string_length_bit_size            /* 0x6f */: /* DWARF5 */
         case DW_AT_string_length_byte_size           /* 0x70 */: /* DWARF5 */
         case DW_AT_rank                              /* 0x71 */: /* DWARF5 */
         case DW_AT_str_offsets_base                  /* 0x72 */: /* DWARF5 */
         case DW_AT_addr_base                         /* 0x73 */: /* DWARF5 */
         case DW_AT_ranges_base                       /* 0x74 */: /* DWARF5 */
         case DW_AT_dwo_id                            /* 0x75 */: /* DWARF5 */
         case DW_AT_dwo_name                          /* 0x76 */: /* DWARF5 */
         case DW_AT_reference                         /* 0x77 */: /* DWARF5 */
         case DW_AT_rvalue_reference                  /* 0x78 */: /* DWARF5 */
         case DW_AT_macros                            /* 0x79 */: /* DWARF5 */
         case DW_AT_call_all_calls                    /* 0x7a */: /* DWARF5 */
         case DW_AT_call_all_source_calls             /* 0x7b */: /* DWARF5 */
         case DW_AT_call_all_tail_calls               /* 0x7c */: /* DWARF5 */
         case DW_AT_call_return_pc                    /* 0x7d */: /* DWARF5 */
         case DW_AT_call_value                        /* 0x7e */: /* DWARF5 */
         case DW_AT_call_origin                       /* 0x7f */: /* DWARF5 */
         case DW_AT_call_parameter                    /* 0x80 */: /* DWARF5 */
         case DW_AT_call_pc                           /* 0x81 */: /* DWARF5 */
         case DW_AT_call_tail_call                    /* 0x82 */: /* DWARF5 */
         case DW_AT_call_target                       /* 0x83 */: /* DWARF5 */
         case DW_AT_call_target_clobbered             /* 0x84 */: /* DWARF5 */
         case DW_AT_call_data_location                /* 0x85 */: /* DWARF5 */
         case DW_AT_call_data_value                   /* 0x86 */: /* DWARF5 */
         case DW_AT_noreturn                          /* 0x87 */: /* DWARF5 */
         case DW_AT_alignment                         /* 0x88 */: /* DWARF5 */
         case DW_AT_export_symbols                    /* 0x89 */: /* DWARF5 */
#endif

/* GNU extensions. */
         case DW_AT_sf_names                          /* 0x2101 */: /* GNU */
         case DW_AT_src_info                          /* 0x2102 */: /* GNU */
         case DW_AT_mac_info                          /* 0x2103 */: /* GNU */
         case DW_AT_src_coords                        /* 0x2104 */: /* GNU */
         case DW_AT_body_begin                        /* 0x2105 */: /* GNU */
         case DW_AT_body_end                          /* 0x2106 */: /* GNU */
         case DW_AT_GNU_vector                        /* 0x2107 */: /* GNU */

/*  Thread safety, see http://gcc.gnu.org/wiki/ThreadSafetyAnnotation .  */
/*  The values here are from gcc-4.6.2 include/dwarf2.h.  The
    values are not given on the web page at all, nor on web pages
    it refers to. */
         case DW_AT_GNU_guarded_by                    /* 0x2108 */: /* GNU */
         case DW_AT_GNU_pt_guarded_by                 /* 0x2109 */: /* GNU */
         case DW_AT_GNU_guarded                       /* 0x210a */: /* GNU */
         case DW_AT_GNU_pt_guarded                    /* 0x210b */: /* GNU */
         case DW_AT_GNU_locks_excluded                /* 0x210c */: /* GNU */
         case DW_AT_GNU_exclusive_locks_required      /* 0x210d */: /* GNU */
         case DW_AT_GNU_shared_locks_required         /* 0x210e */: /* GNU */

/* See http://gcc.gnu.org/wiki/DwarfSeparateTypeInfo */
         case DW_AT_GNU_odr_signature                 /* 0x210f */: /* GNU */

/*  See  See http://gcc.gnu.org/wiki/TemplateParmsDwarf */
/*  The value here is from gcc-4.6.2 include/dwarf2.h.  The value is
    not consistent with the web page as of December 2011. */
         case DW_AT_GNU_template_name                 /* 0x2110 */: /* GNU */
/*  The GNU call site extension.
    See http://www.dwarfstd.org/ShowIssue.php?issue=100909.2&type=open .  */
         case DW_AT_GNU_call_site_value               /* 0x2111 */: /* GNU */
         case DW_AT_GNU_call_site_data_value          /* 0x2112 */: /* GNU */
         case DW_AT_GNU_call_site_target              /* 0x2113 */: /* GNU */
         case DW_AT_GNU_call_site_target_clobbered    /* 0x2114 */: /* GNU */
         case DW_AT_GNU_tail_call                     /* 0x2115 */: /* GNU */
         case DW_AT_GNU_all_tail_call_sites           /* 0x2116 */: /* GNU */
         case DW_AT_GNU_all_call_sites                /* 0x2117 */: /* GNU */
         case DW_AT_GNU_all_source_call_sites         /* 0x2118 */: /* GNU */
         case 0x2119       /* unknown */              /* 0x2119 */:
            break;
			default:
				EMSG("Unsupported attr (CU): 0x%x/%d\n", attr, attr);
         }

      }
      dwarf_dealloc(dbg, attr_list[i], DW_DLA_ATTR);
   }
	dwarf_dealloc(dbg, attr_list, DW_DLA_LIST);
   
      
   return;
}

/** 
 * Processing current DIE
 * 
 * @param dw Dwarf Info structure
 * @param die processed DIE
 * @param lvl level in the tree
 */
void
process_die(dwarf_nfo_t *dw, function_t **funcs, Dwarf_Die die, unsigned long cu_idx)
{

	int         res;
	/*   char       *die_name; */

	/*   Dwarf_Error err; */
	Dwarf_Half  tag;

#if EXPERIMENTAL
	Dwarf_Debug dbg = dw->dbg;
#endif /* EXPERIMENTAL */

	/* BKS Debug purposes */
	Dwarf_Off   offset;

	res = dwarf_tag(die, &tag, NULL);
	if(res != DW_DLV_OK) {
		if(res == DW_DLV_NO_ENTRY) {
			EMSG("Error in DWARF tag() : no entry\n");
			return;
		} else {
			EMSG("Error in DWARF tag()\n");
			return;
		}
	}

	/* BKS Debug purposes */
	res = dwarf_die_CU_offset(die, &offset, NULL);

	switch(tag) {
   case DW_TAG_compile_unit:
      process_compile_unit(dw, die, cu_idx);
      break;
	case DW_TAG_lexical_block:
		break;
	case DW_TAG_subroutine_type:
		break;
	case DW_TAG_inlined_subroutine:
   case DW_TAG_subprogram:
    {
        function_t *func;

        if(dw->curr_func != NULL){
           DMSG("Got a function in a function\n");
           return; /* Skip nested functions for now */
        }
        func = process_subprogram_entry(dw, *funcs, die, (short)tag, cu_idx);
        if(func == NULL){
		    EMSG("<%lu> ERROR of subprogram proccessing\n",
		         (unsigned long)offset);
        }else{
            if(func->declaration == 1){
                //EMSG("Lost function !!!!: %s\n", func->name);
                //function_free(func);
                flist_add_head(funcs, func); 
            }else{
                flist_add_head(funcs, func); 
            }
		}
	}
	break;
	case DW_TAG_variable:
	case DW_TAG_formal_parameter:
	{
      if(dw->curr_func != NULL){
         /* 
          * Only if we are in a function
          * => Handle it in a smarter way later
          */
         symbol_t *sym;
         sym = process_variable_entry(dw, die, cu_idx);
         if(sym == NULL){
            EMSG("ERROR of symbol proccessing\n");
         }else{
            slist_add_head(&dw->curr_func->syms, sym);
         }
      }
	}      
	break;
	case DW_TAG_pointer_type:
	{

	}
	break;

	case DW_TAG_label:
	{
		/* IMSG("<%d> Label :\n", lvl); */

	}
	break;

   case DW_TAG_array_type               /* 0x01 */:
   case DW_TAG_class_type               /* 0x02 */:
   case DW_TAG_entry_point              /* 0x03 */:
   case DW_TAG_enumeration_type         /* 0x04 */:
   /* case DW_TAG_formal_parameter         /\* 0x05 *\/: */
   case DW_TAG_imported_declaration     /* 0x08 */:
   /* case DW_TAG_label                    /\* 0x0a *\/: */
   /* case DW_TAG_lexical_block            /\* 0x0b *\/: */
   case DW_TAG_member                   /* 0x0d */:
   /* case DW_TAG_pointer_type             /\* 0x0f *\/: */
   case DW_TAG_reference_type           /* 0x10 */:
   /* case DW_TAG_compile_unit             /\* 0x11 *\/: */
   case DW_TAG_string_type              /* 0x12 */:
   case DW_TAG_structure_type           /* 0x13 */:
   /* case DW_TAG_subroutine_type          /\* 0x15 *\/: */
   case DW_TAG_typedef                  /* 0x16 */:
   case DW_TAG_union_type               /* 0x17 */:
   case DW_TAG_unspecified_parameters   /* 0x18 */:
   case DW_TAG_variant                  /* 0x19 */:
   case DW_TAG_common_block             /* 0x1a */:
   case DW_TAG_common_inclusion         /* 0x1b */:
   case DW_TAG_inheritance              /* 0x1c */:
   /* case DW_TAG_inlined_subroutine       /\* 0x1d *\/: */
   case DW_TAG_module                   /* 0x1e */:
   case DW_TAG_ptr_to_member_type       /* 0x1f */:
   case DW_TAG_set_type                 /* 0x20 */:
   case DW_TAG_subrange_type            /* 0x21 */:
   case DW_TAG_with_stmt                /* 0x22 */:
   case DW_TAG_access_declaration       /* 0x23 */:
   case DW_TAG_base_type                /* 0x24 */:
   case DW_TAG_catch_block              /* 0x25 */:
   case DW_TAG_const_type               /* 0x26 */:
   case DW_TAG_constant                 /* 0x27 */:
   case DW_TAG_enumerator               /* 0x28 */:
   case DW_TAG_file_type                /* 0x29 */:
   case DW_TAG_friend                   /* 0x2a */:
   case DW_TAG_namelist                 /* 0x2b */:
   case DW_TAG_namelist_item            /* 0x2c */: /* DWARF3/2 spelling */
/* case DW_TAG_namelist_items           0x2c */ /* SGI misspelling/typo */
   case DW_TAG_packed_type              /* 0x2d */:
   /* case DW_TAG_subprogram               /\* 0x2e *\/: */
   case DW_TAG_template_type_parameter  /* 0x2f */: /* DWARF3/2 spelling*/
/* case DW_TAG_template_type_param      0x2f */ /* DWARF2   spelling*/
   case DW_TAG_template_value_parameter /* 0x30 */: /* DWARF3/2 spelling*/
/* case DW_TAG_template_value_param     0x30 */ /* DWARF2   spelling*/
   case DW_TAG_thrown_type              /* 0x31 */:
   case DW_TAG_try_block                /* 0x32 */:
   case DW_TAG_variant_part             /* 0x33 */:
   /* case DW_TAG_variable                 /\* 0x34 *\/: */
   case DW_TAG_volatile_type            /* 0x35 */:
   case DW_TAG_dwarf_procedure          /* 0x36 */:  /* DWARF3 */
   case DW_TAG_restrict_type            /* 0x37 */:  /* DWARF3 */
   case DW_TAG_interface_type           /* 0x38 */:  /* DWARF3 */
   case DW_TAG_namespace                /* 0x39 */:  /* DWARF3 */
   case DW_TAG_imported_module          /* 0x3a */:  /* DWARF3 */
   case DW_TAG_unspecified_type         /* 0x3b */:  /* DWARF3 */
   case DW_TAG_partial_unit             /* 0x3c */:  /* DWARF3 */
   case DW_TAG_imported_unit            /* 0x3d */:  /* DWARF3 */
   case DW_TAG_mutable_type             /* 0x3e */:
   case DW_TAG_condition                /* 0x3f */:  /* DWARF3f */
   case DW_TAG_shared_type              /* 0x40 */:  /* DWARF3f */
   case DW_TAG_type_unit                /* 0x41 */:  /* DWARF4 */
   case DW_TAG_rvalue_reference_type    /* 0x42 */:  /* DWARF4 */
   case DW_TAG_template_alias           /* 0x43 */:  /* DWARF4 */
   case DW_TAG_lo_user                  /* 0x4080 */:

   case DW_TAG_MIPS_loop                /* 0x4081 */:

   case DW_TAG_HP_array_descriptor      /* 0x4090 */: /* HP */

   case DW_TAG_format_label             /* 0x4101 */: /* GNU. Fortran. */
   case DW_TAG_function_template        /* 0x4102 */: /* GNU. For C++ */
   case DW_TAG_class_template           /* 0x4103 */: /* GNU. For C++ */
   case DW_TAG_GNU_BINCL                /* 0x4104 */: /* GNU */
   case DW_TAG_GNU_EINCL                /* 0x4105 */: /* GNU */

   case DW_TAG_GNU_template_template_parameter  /* 0x4106 */: /* GNU */
   /* case DW_TAG_GNU_template_template_param      /\* 0x4106 *\/: /\* GNU *\/ */
   case DW_TAG_GNU_template_parameter_pack      /* 0x4107 */: /* GNU */
   case DW_TAG_GNU_formal_parameter_pack        /* 0x4108 */: /* GNU */
      
   case DW_TAG_GNU_call_site                    /* 0x4109 */: /* GNU */
   case DW_TAG_GNU_call_site_parameter          /* 0x410a */: /* GNU */
      
   case DW_TAG_ALTIUM_circ_type         /* 0x5101 */: /* ALTIUM */
   case DW_TAG_ALTIUM_mwa_circ_type     /* 0x5102 */: /* ALTIUM */
   case DW_TAG_ALTIUM_rev_carry_type    /* 0x5103 */: /* ALTIUM */
   case DW_TAG_ALTIUM_rom               /* 0x5111 */: /* ALTIUM */
      
   case DW_TAG_upc_shared_type          /* 0x8765 */: /* UPC */
   case DW_TAG_upc_strict_type          /* 0x8766 */: /* UPC */
   case DW_TAG_upc_relaxed_type         /* 0x8767 */: /* UPC */
   case DW_TAG_PGI_kanji_type           /* 0xa000 */: /* PGI */
   case DW_TAG_PGI_interface_block      /* 0xa020 */: /* PGI */
   case DW_TAG_SUN_function_template    /* 0x4201 */: /* SUN */
   case DW_TAG_SUN_class_template       /* 0x4202 */: /* SUN */
   case DW_TAG_SUN_struct_template      /* 0x4203 */: /* SUN */
   case DW_TAG_SUN_union_template       /* 0x4204 */: /* SUN */
   case DW_TAG_SUN_indirect_inheritance /* 0x4205 */: /* SUN */
   case DW_TAG_SUN_codeflags            /* 0x4206 */: /* SUN */
   case DW_TAG_SUN_memop_info           /* 0x4207 */: /* SUN */
   case DW_TAG_SUN_omp_child_func       /* 0x4208 */: /* SUN */
   case DW_TAG_SUN_rtti_descriptor      /* 0x4209 */: /* SUN */
   case DW_TAG_SUN_dtor_info            /* 0x420a */: /* SUN */
   case DW_TAG_SUN_dtor                 /* 0x420b */: /* SUN */
   case DW_TAG_SUN_f90_interface        /* 0x420c */: /* SUN */
   case DW_TAG_SUN_fortran_vax_structure /* 0x420d */: /* SUN */
   case DW_TAG_SUN_hi                   /* 0x42ff */: /* SUN */
   case DW_TAG_hi_user                  /* 0xffff */:

      break;

	default:
    
		EMSG("Unsupported TAG : 0x%x/%d\n", tag, tag);

	}
	/*   show_attr(dbg, die); */
  
	return;
}

/** 
 * Walk through the Dwarf2 tree
 * 
 * @param [in]dw dwarf info structure
 * @param [out]funcs empty structure with functions to be filled
 * @param [in]die tree root
 * @param [in]level level of the current walk
 */
void
dwarf_walk(dwarf_nfo_t *dw, function_t **funcs, Dwarf_Die die,
           unsigned long cu_idx, int no_child)
{

	int         res;
	Dwarf_Die   next_die;
	Dwarf_Debug dbg = dw->dbg;

	/* first process current die */
	if(die != NULL)
		process_die(dw, funcs, die, cu_idx);
  
	/* second, look for child */
	if(die != NULL && no_child != 1){
		res = dwarf_child(die, &next_die, NULL);

		if(res == DW_DLV_OK){
			dwarf_walk(dw, funcs, next_die, cu_idx, no_child);
		}
	}

	/* third, process sibling ... */
	res = dwarf_siblingof(dbg, die, &next_die, NULL);
	if(res == DW_DLV_OK){
		dwarf_walk(dw, funcs, next_die, cu_idx, no_child);
	}
  
	return;
}

/** 
 * Retrieve the location for a variable
 * 
 * @param dw Dwarf2 Info to populate
 * @param addr PC of the lookup
 * @param sym symbol looked up
 */
int
dwarf_nfo_get_loc(dwarf_nfo_t *dw, function_t *funcs, uint32_t addr, char *sym,
                  Dwarf_Locdesc **loc, Dwarf_Locdesc **frame_base){

   (void)dw;
   int i = 0;
   symbol_t *res = NULL;
   function_t *p = funcs;
   Dwarf_Locdesc *fb_loc = NULL;
   
   while(p){
      /*
       * Look for framebase first
       */
      for(i = 0; i < p->nb_fb_locs; i++){
         if(IS_IN_LOCATION_RANGE(addr, p->fb_locs[i])){
            fb_loc = res->locs[i];
            break;
         }
      }

      res = slist_lookup_name(p->syms, sym);
#ifdef DEBUG
      IMSG("Looking for sym: %s\n", sym);
      IMSG("in\n");
      slist_dump(p->syms);
#endif
      if(res){
         break;
      }
      p = p->next;
   }

   *frame_base = fb_loc;
   
   if(res == NULL){
      return DBG_NFO_NO_SYM;
   }
    
   if(res->nb_locs == 0)
      return DBG_NFO_NO_LOC;
   
   if(res->nb_locs == 1){
      *loc = res->locs[0];
      return DBG_NFO_LOC_FOUND;
   }
      
   for(i = 0; i < res->nb_locs; i++){
      if(IS_IN_LOCATION_RANGE(addr, res->locs[i])){
         *loc = res->locs[i];
         return DBG_NFO_LOC_FOUND;
      }
   }
   DMSG("Not in a range\n");
   return DBG_NFO_NO_LOC;
}
   
/** 
 * Populate the functions contexts information
 * in the Dwarf2 Info structure given by parameter
 * 
 * @param dw Dwarf2 Info to populate
 * @param funcs
 */
void
dwarf_nfo_process_funcs(dwarf_nfo_t *dw, function_t **funcs){

	int            res;
	int            the_end = 0;
	unsigned long  cu_idx = 0;

	Dwarf_Unsigned cu_hdr_len;
	Dwarf_Half     ver_stamp;
	Dwarf_Unsigned abbr_off;
	Dwarf_Half     addr_size;
	Dwarf_Unsigned next_cu_hdr;

	do{
    
		res = dwarf_next_cu_header(dw->dbg, &cu_hdr_len, &ver_stamp, 
		                           &abbr_off, &addr_size, &next_cu_hdr, NULL);
		if( res != DW_DLV_OK ){
			if( res == DW_DLV_NO_ENTRY){
				HMSG("No more CU\n");
				break;
			}else{
				EMSG("Error in DWARF next_cu_header()\n");
				exit(EXIT_FAILURE);
			}
			the_end = 1;
		}else{
      
			HMSG("<%lu> CU header len : %u\n", cu_idx, (unsigned int)cu_hdr_len);
      
			dwarf_walk(dw, funcs, NULL, cu_idx, 0);
      
		}
	}
	while(!the_end);

  
	return;
}

/** 
 * Process all Dwarf2 Info available in the binary file.
 * - Functions context.
 * - Source files and lines.
 * 
 * @param dw 
 */
#if 0
void
dwarf_nfo_process(dwarf_nfo_t *dw, Elf *elf){


	dwarf_nfo_process_src(dw);
	dwarf_nfo_functions(dw);

	return;
}
#endif

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
