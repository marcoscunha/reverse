#include <err.h>
#include <string.h>
#include <stdlib.h>

#include "extract_lock_descr.h"

/*
 * lock_tab allocated by extract_lock_descr
 */
int extract_lock_descr(Elf *e, char *section_name, lock_descr_t **lock_tab, int *nb_entry)
{
  int ret;
  char *name;
  Elf_Scn *scn = NULL;
  GElf_Shdr shdr;
  size_t shstrndx;

  /* Get section header string */
  if (elf_getshdrstrndx (e, &shstrndx) != 0)
    errx (EXIT_FAILURE, "elf_getshdrstrndx() failed : %s.", elf_errmsg(-1));

  while ((scn = elf_nextscn(e, scn)) != NULL) {
    if (gelf_getshdr(scn, &shdr) != &shdr)
      errx(EXIT_FAILURE, " getshdr() failed : %s.", elf_errmsg(-1));
    if ((name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL )
      errx(EXIT_FAILURE, "elf_strptr() failed : %s.", elf_errmsg(-1));

    if (strcmp(name, section_name) == 0) {
      ret = extract_lock_descr_from_section(scn, lock_tab, nb_entry);
      break;
    }
  }

  return ret;
}

int extract_lock_descr_from_section(Elf_Scn *scn, lock_descr_t **lock_tab, int *nb_entry)
{
  *nb_entry = 0;
  if (scn == NULL) {
    return -1;
  }

  Elf_Data *data = NULL;
  magic_lock_descr_t *lock_annot = NULL;
  int current_lock = 0;
  int lock_entry = 0;

  while ((data = elf_getdata(scn, data)) != NULL) {
    lock_annot = (magic_lock_descr_t*)data->d_buf;
    int nb_lock_annot = data->d_size/sizeof(magic_lock_descr_t);
    if (nb_lock_annot*sizeof(magic_lock_descr_t) < data->d_size) {
      nb_lock_annot++;
    }
    *nb_entry += nb_lock_annot;
    *lock_tab = (lock_descr_t*)realloc(*lock_tab, (*nb_entry)*sizeof(lock_descr_t));
    /* iter in the current data */
    for (current_lock=0 ; current_lock<nb_lock_annot ; current_lock++) {
      (*lock_tab)[lock_entry++].magic_lock_descr = lock_annot[current_lock];
      //(*lock_tab)[lock_entry++].lock_location = 0;
    }
  }

  return 0;
}
