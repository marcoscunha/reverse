#ifndef __FAT_OPTS_H__
#define __FAT_OPTS_H__

/*------------------------------------------------------------- */
/* Configuration */
/*------------------------------------------------------------- */

/* Is the system little endian (1) or big endian (0) */
#define FATFS_IS_LITTLE_ENDIAN				1

/* Max FAT sectors to buffer (min 1) */
/* (mem used is FAT_BUFFERED_SECTORS * FAT_SECTOR_SIZE) */
#define FAT_BUFFERED_SECTORS				1

/* Sector size used */
#define FAT_SECTOR_SIZE						512

#endif
