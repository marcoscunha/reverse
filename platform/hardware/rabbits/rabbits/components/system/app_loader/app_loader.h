#ifndef __APP_LOADER_H__
#define __APP_LOADER_H__

#include <generic_subsystem.h>

#include <libelf.h>
#include <gelf.h>

typedef struct linux_args linux_args_t;
struct linux_args {
	 uint32_t  linux_board_id;
	 uint32_t  atag_loadaddr;
	 
	 uint32_t  ram_startaddr;
	 int       ram_size;
	 
	 uint32_t  kernel_loadaddr;
	 char     *kernel_fname;
	 
	 uint32_t  initrd_loadaddr;
	 char     *initrd_fname;
	 uint32_t  initrd_size;
	 
	 char     *kernel_cmdline;

	 uint32_t  bootloader_loadaddr;
	 uint32_t  secondary_bootaddr;
};

#define APPLOADER_DEFAULT (0xFFFFFFFFUL)

class app_loader {

private:
	 int                m_elf_fd;
	 generic_subsystem *m_subsystem;

public:
	 app_loader(generic_subsystem *subsys);
	 ~app_loader(void);
	 int load_elf_image(const char *elf_fname, uint32_t mast_id, uint32_t load);
	 int load_binary_image(const char *file, uint32_t mast_id, uint32_t load);
	 void load_linux_image(linux_args_t *args, uint32_t mast_id);

	 void set_linux_kernel_args(linux_args_t *args, uint32_t mast_id);
private:
	 int load_elf_segments(Elf *elf, uint32_t mast_id, uint32_t load);

	 int read_shdr(Elf *elf);
	 int read_phdr(Elf *elf);

	 static uint32_t bootloader[];
	 static uint32_t smpboot[];
	 
};

#endif /* __APP_LOADER_H__ */
