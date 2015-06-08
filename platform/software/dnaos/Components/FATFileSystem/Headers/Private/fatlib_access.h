#ifndef __FAT_ACCESS_H__
#define __FAT_ACCESS_H__

#include <Private/FATFileSystem.h>

/*----------------------------------------------------------------------------- */
/* Prototypes */
/*----------------------------------------------------------------------------- */
int16_t		fatfs_init(struct fatfs *fs);
uint32_t	fatfs_lba_of_cluster(struct fatfs *fs, uint32_t Cluster_Number);
int16_t		fatfs_sector_reader(struct fatfs *fs, uint32_t Startcluster, uint32_t offset, unsigned char *target);
uint32_t	fatfs_get_root_cluster(struct fatfs *fs);

#endif

