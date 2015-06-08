#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <gelf.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "extract_lock_descr.h"
#include "print_lock_descr.h"
#include "get_dwarf_location.h"

int main(int argc, char **argv)
{
  int fd;
  Elf *e;
  lock_descr_t *lock_tab = NULL;
  int nb_entry = 0;

  if (argc < 3)
    errx ( EXIT_FAILURE , " usage : %s file_name section_name1 [sct_name2 ...]" , argv [0]);
  char *elf = argv[1];
  /* Init and open of the elf file */
  if ( elf_version ( EV_CURRENT ) == EV_NONE )
    errx ( EXIT_FAILURE , " ELF library initialization  failed : %s." , elf_errmsg ( -1));
  if (( fd = open ( elf , O_RDONLY , 0)) < 0)
    errx ( EXIT_FAILURE , " open \" %s \" failed " , elf);
  if (( e = elf_begin ( fd , ELF_C_READ , NULL )) == NULL )
    errx ( EXIT_FAILURE , " elf_begin () failed : %s.", elf_errmsg ( -1));
  if ( elf_kind ( e ) != ELF_K_ELF ) 
    errx ( EXIT_FAILURE , " %s is not an ELF object.", elf);
  /* init debug helper */
  debug_helper_t *dh = dh_init(elf);

  int i;
  for (i=2 ; i<argc ; i++) {
    extract_lock_descr(e, argv[i], &lock_tab, &nb_entry);
    get_dwarf_location(dh, lock_tab, nb_entry);
    printf("%s:\n", argv[i]);
    print_lock_descr(lock_tab, nb_entry);
    printf("\n");
    free(lock_tab);
    lock_tab = NULL;
    nb_entry = 0;
  }

  /* close */
  dh_free(dh);
  elf_end(e);
  close(fd);
  return 0;
}

