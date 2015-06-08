#ifndef __FAT_MISC_H__
#define __FAT_MISC_H__

#include <Private/fatlib_defs.h>
#include <Private/fatlib_opts.h>

/*----------------------------------------------------------------------------- */
/* Macros */
/*----------------------------------------------------------------------------- */
/* Little Endian */
#if FATFS_IS_LITTLE_ENDIAN
	#define GET_32BIT_WORD(buffer, location)	( ((uint32_t)buffer[location+3]<<24) + ((uint32_t)buffer[location+2]<<16) + ((uint32_t)buffer[location+1]<<8) + (uint32_t)buffer[location+0] )
	#define GET_16BIT_WORD(buffer, location)	( ((uint16_t)buffer[location+1]<<8) + (uint16_t)buffer[location+0] )

	#define SET_32BIT_WORD(buffer, location, value)	{ buffer[location+0] = (unsigned char)((value)&0xFF); \
													  buffer[location+1] = (unsigned char)((value>>8)&0xFF); \
													  buffer[location+2] = (unsigned char)((value>>16)&0xFF); \
													  buffer[location+3] = (unsigned char)((value>>24)&0xFF); }

	#define SET_16BIT_WORD(buffer, location, value)	{ buffer[location+0] = (unsigned char)((value)&0xFF); \
													  buffer[location+1] = (unsigned char)((value>>8)&0xFF); }
/* Big Endian */
#else
	#define GET_32BIT_WORD(buffer, location)	( ((uint32_t)buffer[location+0]<<24) + ((uint32_t)buffer[location+1]<<16) + ((uint32_t)buffer[location+2]<<8) + (uint32_t)buffer[location+3] )
	#define GET_16BIT_WORD(buffer, location)	( ((uint16_t)buffer[location+0]<<8) + (uint16_t)buffer[location+1] )

	#define SET_32BIT_WORD(buffer, location, value)	{ buffer[location+3] = (unsigned char)((value)&0xFF); \
													  buffer[location+2] = (unsigned char)((value>>8)&0xFF); \
													  buffer[location+1] = (unsigned char)((value>>16)&0xFF); \
													  buffer[location+0] = (unsigned char)((value>>24)&0xFF); }

	#define SET_16BIT_WORD(buffer, location, value)	{ buffer[location+1] = (unsigned char)((value)&0xFF); \
													  buffer[location+0] = (unsigned char)((value>>8)&0xFF); }
#endif

/*----------------------------------------------------------------------------- */
/* Prototypes */
/*----------------------------------------------------------------------------- */
int16_t	fatfs_entry_lfn_text(fatfs_entry_t entry);
int16_t	fatfs_entry_lfn_invalid(fatfs_entry_t entry);
int16_t	fatfs_entry_lfn_exists(fatfs_entry_t entry);
int16_t	fatfs_entry_sfn_only(fatfs_entry_t entry);
int16_t	fatfs_entry_is_dir(fatfs_entry_t entry);
//int16_t	fatfs_entry_is_file(fatfs_entry_t entry);
void	fatfs_lfn_entry(unsigned char *entryBuffer, char *LongFilename);
void 	fatfs_reset_lfn(char *filename);
int16_t	fatfs_get_fn_entry(fatfs_entry_t directoryEntry, char *filename);
int16_t fatfs_get_sfn_from_entry(fatfs_entry_t directoryEntry, char *ShortFilename);
int32_t fatfs_compare_names(char* s1, char* s2);

#endif

