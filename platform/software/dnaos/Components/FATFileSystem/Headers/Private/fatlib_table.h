#ifndef __FAT_TABLE_H__
#define __FAT_TABLE_H__

#include <Private/fatlib_opts.h>
#include <Private/fatlib_misc.h>

/*----------------------------------------------------------------------------- */
/* Prototypes */
/*----------------------------------------------------------------------------- */
void	fatfs_fat_init(struct fatfs *fs);
uint32_t	fatfs_find_next_cluster(struct fatfs *fs, uint32_t current_cluster);
void	fatfs_set_fs_info_next_free_cluster(struct fatfs *fs, uint32_t newValue);

#endif
