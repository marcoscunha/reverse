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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gelf.h>

#include <debug_nfo.h>

#ifdef DEBUG_NFO_DEBUG
#define DEBUG
#endif

#define DBG_HDR "debug_nfo"
#include <debug.h>

debug_nfo_t *
debug_nfo_init(char *fname)
{
   debug_nfo_t *res = malloc(sizeof(debug_nfo_t));

   res->exec_fname = malloc(strlen(fname)+1);
   strcpy(res->exec_fname, fname);

   res->exec_fd  = 0;
   res->elf      = NULL;
   res->dwarfs   = NULL;
   res->funcs    = NULL;
   res->src_tabs = NULL;

   return res;
}

void
debug_nfo_free_src_lines(src_tab_t *lines){

   src_tab_t *p = lines;
   while(p){
      src_tab_t *freed = p;
      int i = 0;
      p = p->next;

      for(i = 0; i < freed->nb_files; i++){
         free(freed->files[i]);
      }
      free(freed->files);
      free(freed->lines);
      free(freed);
   }
   return;
}

void
debug_nfo_free_functions(function_t *funcs){

   flist_free(funcs);
   return;
}


void
debug_nfo_closure(debug_nfo_t *dbg)
{
   elf_end(dbg->elf);
   close(dbg->exec_fd);

   debug_nfo_free_functions(dbg->funcs);
   debug_nfo_free_src_lines(dbg->src_tabs);

   /*
    * Keep dwarf infos ... for locations
    */
   dwarf_nfo_closure(dbg->dwarfs);

   free(dbg->exec_fname);

   free(dbg);
   HMSG("Dbg_nfo Closure done.\n");

   return;
}

#if 0
/* Given Elf header, Elf_Scn, and Elf32_Shdr 
 * print out the symbol table 
 */
print_symbols(Elf *elf, Elf_Scn *scn, Elf32_Shdr *shdr)
{
   Elf_Data *data;
   char *name;
   char *stringName;
   data = 0;
   int number = 0;
   if ((data = elf_getdata(scn, data)) == 0 || data->d_size == 0){
      /* error or no data */
      fprintf(stderr,"Section had no data!n");
      exit(-1);
   }
   /*now print the symbols*/
   Elf32_Sym *esym = (Elf32_Sym*) data->d_buf;
   Elf32_Sym *lastsym = (Elf32_Sym*) ((char*) data->d_buf + data->d_size);
   /* now loop through the symbol table and print it*/
   for (; esym < lastsym; esym++){
      if ((esym->st_value == 0) ||
          (ELF32_ST_BIND(esym->st_info)== STB_WEAK) ||
          (ELF32_ST_BIND(esym->st_info)== STB_NUM) ||
          (ELF32_ST_TYPE(esym->st_info)!= STT_FUNC)) 
         continue;
      name = elf_strptr(elf,shdr->sh_link , (size_t)esym->st_name);
      if(!name){
         fprintf(stderr,"%sn",elf_errmsg(elf_errno()));
         exit(-1);
      }
      printf("%d: %sn",number++, name);
   }
}
#endif

void
symtab_process(debug_nfo_t *dbg, function_t **funcs)
{

   Elf_Scn* section = 0;
   while ((section = elf_nextscn(dbg->elf, section)) != 0) {
      char       *name = 0;
      int         i    = 0;
      GElf_Shdr   shdr;
      Elf_Data   *data    = NULL;
      int         sym_count   = 0;
      
      gelf_getshdr (section, &shdr);

      if (shdr.sh_type != SHT_SYMTAB) {
         continue;
      }
      sym_count = shdr.sh_size / shdr.sh_entsize;
      data = elf_getdata(section, data);

      if (sym_count == 0){
         fprintf(stderr,"Section had no Symbols\n");
      } /* if (count == 0) */

      /*
       * First round: global and local functions
       */
      for(i = 0; i < sym_count; i++){

         GElf_Sym esym;
         gelf_getsym(data, i, &esym);

         if ((ELF32_ST_BIND(esym.st_info) == STB_WEAK) ||
             (ELF32_ST_BIND(esym.st_info) == STB_NUM)  ||
             (ELF32_ST_TYPE(esym.st_info) != STT_FUNC) ){ 

            name = elf_strptr(dbg->elf, shdr.sh_link, (size_t)esym.st_name);

#if 0
            HMSG(" skipped sym: %s\n", name);
#endif /* 0 */

            continue;
         } /* if( (esym.st_value == 0) [...] STT_FUNC) ) */

         name = elf_strptr(dbg->elf, shdr.sh_link, (size_t)esym.st_name);
         if(!name){
            EMSG("%s\n", elf_errmsg(elf_errno()));
            return;
         }

         function_t *found_func = NULL;
         found_func = flist_lookup_entry(*funcs, (unsigned long)esym.st_value);
         if( found_func                                     &&
             ((found_func->type == FUNCTION_TYPE_INLINE) ||
              (found_func->declaration == 1)              ) ){
             HMSG("excluded  : @ [0x%08x - 0x%08x] %s\n",
                                 (uint32_t)esym.st_value,
                                 (uint32_t)(esym.st_size + esym.st_value),
                                 name);
             /* Ignore what we found */
             found_func = NULL;
         }
         if(found_func) {
            if(strcmp(name, found_func->name) == 0){
               HMSG("found     : @ [0x%08x - 0x%08x] %s\n",
                    (uint32_t)esym.st_value,
                    (uint32_t)(esym.st_size + esym.st_value),
                    name);
            }else{
               DMSG("found !!! : @ [0x%08x - 0x%08x] sym(%s) != found(%s)\n",
                    (uint32_t)esym.st_value,
                    (uint32_t)(esym.st_size + esym.st_value),
                    name, found_func->name);
            }
         } else {
            function_t *new_func = function_new();
            new_func->type   = FUNCTION_TYPE_FUNC;
            new_func->lowpc  = (uint32_t)esym.st_value;

            if(esym.st_size != 0) {
               new_func->highpc = (uint32_t)esym.st_size + esym.st_value;
            }else{
               new_func->highpc = (uint32_t)0xFFFFFFFF;
               /* new_func->highpc = (uint32_t)esym.st_value; */
            } /* if(esym.st_size != 0) */

            new_func->name   = malloc(strlen(name)+1);
            strcpy(new_func->name, name);
            new_func->decl_file = 0;
            new_func->decl_line = 0;

            flist_add_head(funcs, new_func); 

            HMSG("rtrvd     : @ [0x%08x - 0x%08x] %s\n",
                 new_func->lowpc, new_func->highpc, name);
         } /* if(found_func) */
      } /* for(; esym < lastsym; esym++) */

      /*
       * A second round for Weak symbols
       */
      for(i = 0; i < sym_count; i++){

         GElf_Sym esym;
         gelf_getsym(data, i, &esym);
         if( (ELF32_ST_BIND(esym.st_info) == STB_WEAK) &&
             (ELF32_ST_TYPE(esym.st_info) == STT_FUNC) ){

            name = elf_strptr(dbg->elf, shdr.sh_link, (size_t)esym.st_name);

            if(!name){
               EMSG("%s\n", elf_errmsg(elf_errno()));
               return;
            }
            function_t *found_func = NULL;
            found_func = flist_lookup_entry(*funcs, (unsigned long)esym.st_value);
            if(found_func) {
               HMSG("found weak: @ [0x%08x - 0x%08x] %s\n",
                    (uint32_t)esym.st_value,
                    (uint32_t)(esym.st_size + esym.st_value),
                    name);
            } else {
               found_func = flist_lookup_name(*funcs, name);
               if(!found_func) {
                  function_t *new_func = function_new();
                  new_func->type   = FUNCTION_TYPE_FUNC;
                  new_func->lowpc  = (uint32_t)esym.st_value;

                  if(esym.st_size != 0) {
                     new_func->highpc = (uint32_t)esym.st_size + esym.st_value;
                  }else{
                     new_func->highpc = (uint32_t)0xFFFFFFFF;
                     /* new_func->highpc = (uint32_t)esym.st_value; */
                  } /* if(esym.st_size != 0) */
                  new_func->name   = malloc(strlen(name)+1);
                  strcpy(new_func->name, name);
                  new_func->decl_file = 0;
                  new_func->decl_line = 0;

                  flist_add_head(funcs, new_func); 

                  HMSG("rtrvd weak: @ [0x%08x - 0x%08x] %s\n",
                       new_func->lowpc, new_func->highpc, name);

               } /* if(!found_func) */
            } /* if(found_func) */

         } /* if( (ELF32_ST_BIND(esym.st_info) == STB_WEAK) && (ELF32_ST_TYPE(esym.st_info) == STT_FUNC) ) */

      } /* for (; esym < lastsym; esym++){ */

   } /* while ((section = elf_nextscn(dbg->elf, section)) != 0) */

   /* no symtab */
   return;
}

int
debug_nfo_get_loc(debug_nfo_t *dbg, uint32_t addr, char *sym,
                  Dwarf_Locdesc **loc, Dwarf_Locdesc **frame_base,
                  function_t *func){

   if(func == NULL){
      return DBG_NFO_NO_FUNC;
   }
   return dwarf_nfo_get_loc(dbg->dwarfs, func, addr, sym, loc, frame_base);
}


void
debug_nfo_process(debug_nfo_t *dbg)
{

   if(elf_version(EV_CURRENT) == EV_NONE) {
      /* library out of date */
      EMSG("ELF library is out of date\n");
      exit(EXIT_FAILURE);
   }

   /* opening executable file */
   dbg->exec_fd = open(dbg->exec_fname, O_RDONLY);
   if(dbg->exec_fd < 0) {
      EMSG("Unable to open file : %s\n", dbg->exec_fname);
      exit(EXIT_FAILURE);
   }

   /* Elf init */
   dbg->elf = elf_begin(dbg->exec_fd, ELF_C_READ, NULL);

   if( dbg->elf == NULL ) {
      EMSG("Unable to load Elf file\n");
      exit(EXIT_FAILURE);
   }
                                  	
   /* Dwarf init */
   dbg->dwarfs = dwarf_nfo_init(dbg->elf);
                                  	
   if(dbg->dwarfs){
      /* Parse DWARFs to get infos */
      dwarf_nfo_process_src(dbg->dwarfs, &(dbg->src_tabs));
      dwarf_nfo_process_funcs(dbg->dwarfs, &(dbg->funcs));
   }

   /* Parse Symtab to complete */
   symtab_process(dbg, &(dbg->funcs));

   return;
}


void
debug_nfo_dump_src(debug_nfo_t *dbg)
{
   int i;
   src_tab_t *tab = dbg->src_tabs;

   while(tab) {
      line_nfo_t *line = tab->lines;

      for( i=0;i< tab->nb_lines;i++,line++){

         printf(" @0x%08x \n", (unsigned int)line->addr);

      }
      tab = tab->next;
   }
   /*   printf(" @0x%x",(unsigned int)lineaddr); */
   /*   if(block_begin){ */
   /*     printf(" bb "); */
   /*   }else{ */
   /*     printf("    "); */
   /*   } */
   /*   printf("[%u:", (unsigned int)lineno); */
   /*   if( fileno > 0 && fileno <= (Dwarf_Unsigned)file_cnt ){ */
   /*     printf("%s]\n", file_buffer[fileno-1]); */
   /*   }else{ */
   /*     printf("ERROR %d]\n", (unsigned int)fileno); */
   /*   } */

   return;
}

void
debug_nfo_dump_func(debug_nfo_t *dbg)
{
   flist_dump(dbg->funcs);
   return;
}

/*
 * Local Variables:
 * mode: c
 * tab-width: 3
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
