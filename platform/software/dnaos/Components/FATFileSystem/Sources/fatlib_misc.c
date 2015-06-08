/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
					        FAT16/32 File IO Library
								    V2.6
 	  							 Rob Riglar
						    Copyright 2003 - 2010

   					  Email: rob@robriglar.com

								License: GPL
   If you would like a version with a more permissive license for use in
   closed source commercial applications please contact me for details.
-----------------------------------------------------------------------------

 This file is part of FAT File IO Library.

 FAT File IO Library is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 FAT File IO Library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with FAT File IO Library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
-----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------- */

#include <Private/FATFileSystem.h>
#include <Private/fatlib_misc.h>

/*----------------------------------------------------------------------------- */
/* fatfs_reset_lf: Clear long file name */
/*----------------------------------------------------------------------------- */
void fatfs_reset_lfn(char *filename)
{
	uint16_t i;
	
	for(i = 0; i < 261; i++)
		filename[i] = '\0';
}

/*----------------------------------------------------------------------------- */
/* fatfs_lfn_entry - Function extracts long file name text from sector */
/* at a specific offset */
/*----------------------------------------------------------------------------- */
void fatfs_lfn_entry(unsigned char *entryBuffer, char *LongFilename)
{
	uint8_t LFNIndex, i;
	
	LFNIndex = ((entryBuffer[0] & 0x0F) - 1)*13;

	LongFilename[LFNIndex] 		= entryBuffer[0x01];
	LongFilename[LFNIndex+1] 	= entryBuffer[0x03];
	LongFilename[LFNIndex+2] 	= entryBuffer[0x05];
	LongFilename[LFNIndex+3] 	= entryBuffer[0x07];	
	LongFilename[LFNIndex+4] 	= entryBuffer[0x09];
	LongFilename[LFNIndex+5] 	= entryBuffer[0x0E];
	LongFilename[LFNIndex+6] 	= entryBuffer[0x10];
	LongFilename[LFNIndex+7] 	= entryBuffer[0x12];
	LongFilename[LFNIndex+8] 	= entryBuffer[0x14];
	LongFilename[LFNIndex+9] 	= entryBuffer[0x16];
	LongFilename[LFNIndex+10] 	= entryBuffer[0x18];
	LongFilename[LFNIndex+11] 	= entryBuffer[0x1C];
	LongFilename[LFNIndex+12] 	= entryBuffer[0x1E];
	
	for (i = 0; i < 13; i++)
		if (LongFilename[LFNIndex+i] == 0xFF)
			LongFilename[LFNIndex+i] = 0x20;
}

/*----------------------------------------------------------------------------- */
/* fatfs_entry_lfn_text: If LFN text entry found */
/*----------------------------------------------------------------------------- */
int16_t fatfs_entry_lfn_text(fatfs_entry_t entry)
{
	return ((entry->Attr & FILE_ATTR_LFN_TEXT) == FILE_ATTR_LFN_TEXT);
}
/*----------------------------------------------------------------------------- */
/* fatfs_entry_lfn_invalid: If SFN found not relating to LFN */
/*----------------------------------------------------------------------------- */
int16_t fatfs_entry_lfn_invalid(fatfs_entry_t entry)
{
	return ( (entry->Name[0]==FILE_HEADER_BLANK)  ||
		 (entry->Name[0]==FILE_HEADER_DELETED)||
		 (entry->Attr==FILE_ATTR_VOLUME_ID) ||
		 (entry->Attr & FILE_ATTR_SYSHID) );
}

/*----------------------------------------------------------------------------- */
/* fatfs_entry_lfn_exists: If LFN exists and correlation SFN found */
/*----------------------------------------------------------------------------- */
int16_t fatfs_entry_lfn_exists(fatfs_entry_t entry)
{
	return ( (entry->Attr!=FILE_ATTR_LFN_TEXT) &&
		 (entry->Name[0]!=FILE_HEADER_BLANK) &&
		 (entry->Name[0]!=FILE_HEADER_DELETED) &&
		 (entry->Attr!=FILE_ATTR_VOLUME_ID) &&
		 (!(entry->Attr&FILE_ATTR_SYSHID)) );
}

/*----------------------------------------------------------------------------- */
/* fatfs_entry_sfn_only: If SFN only exists */
/*----------------------------------------------------------------------------- */
int16_t fatfs_entry_sfn_only(fatfs_entry_t entry)
{
	return ( (entry->Attr!=FILE_ATTR_LFN_TEXT) &&
		 (entry->Name[0]!=FILE_HEADER_BLANK) &&
		 (entry->Name[0]!=FILE_HEADER_DELETED) &&
		 (entry->Attr!=FILE_ATTR_VOLUME_ID) &&
		 (!(entry->Attr&FILE_ATTR_SYSHID)) );
}

/*----------------------------------------------------------------------------- */
/* fatfs_entry_is_dir: Returns 1 if a directory */
/*----------------------------------------------------------------------------- */
int16_t fatfs_entry_is_dir(fatfs_entry_t entry)
{
	return ((entry->Attr & FILE_TYPE_DIR) != 0);
}

/*----------------------------------------------------------------------------- */
/* fatfs_entry_is_file: Returns 1 is a file entry */
/*----------------------------------------------------------------------------- */
#if 0
int16_t fatfs_entry_is_file(fatfs_entry_t entry)
{
	return ( (entry->Attr & FILE_TYPE_FILE) != 0);
}
#endif

/*----------------------------------------------------------------------------- */
/* fatfs_get_fn_entry: */
/*----------------------------------------------------------------------------- */
int16_t fatfs_get_fn_entry(fatfs_entry_t directoryEntry, char *filename)
{
	/* Long File Name Text Found */
	if (fatfs_entry_lfn_text(directoryEntry) )
		fatfs_lfn_entry((unsigned char*)directoryEntry, filename);

	/* If Invalid record found delete any long file name 
		information collated */
	else if (fatfs_entry_lfn_invalid(directoryEntry) )
		fatfs_reset_lfn(filename);

	/* Normal SFN Entry and Long text exists  */
	else if ((fatfs_entry_lfn_exists(directoryEntry) && dna_strlen(filename) != 0) 
		|| (fatfs_get_sfn_from_entry(directoryEntry, filename) == 0))
		return 0;

	return -1;
}

/*----------------------------------------------------------------------------- */
/* get_sfn_from_entry: */
/*----------------------------------------------------------------------------- */
int16_t fatfs_get_sfn_from_entry(fatfs_entry_t directoryEntry, char *filename)
{
    uint8_t i, j;
	
	/* Normal Entry, only 8.3 Text		 */
	if(fatfs_entry_sfn_only(directoryEntry))
	{
		fatfs_reset_lfn(filename);
	
		for (i = 0; i < 8; i++)
		{
			if(directoryEntry->Name[i] == ' ')
				break;
			filename[i] = directoryEntry->Name[i];
		}

		if(filename[0] != '.' && directoryEntry->Name[8] != ' ')
			filename[i] = '.';
			
		for(j = 0; j < 3; j++)
		{
			if(directoryEntry->Name[8+j] != ' ')
				filename[i++] = directoryEntry->Name[8+j];
		}
		
		filename[i] = '\0';
		
		return 0;
	}
	
	return -1;
}

/*----------------------------------------------------------------------------- */
/* fatfs_compare_names : nocase sensible string comparaison */
/*----------------------------------------------------------------------------- */

static char fatfs_toupper(char c)
{
	return ((c >= 'a') && (c <= 'z')) ? c - 32 : c;
}


int32_t fatfs_compare_names(char* s1, char* s2)
{
  char *s1_ptr = (char *)s1, *s2_ptr = (char *)s2;
	
  do
  {
    if (fatfs_toupper(*s1_ptr++) != fatfs_toupper(*s2_ptr))
    {
      return -1;
    }
  }
  while (*s2_ptr++ != '\0');
  
  return 0;
}

/*
 ****/


