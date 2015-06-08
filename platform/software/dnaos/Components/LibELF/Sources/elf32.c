/*
 * Australian Public Licence B (OZPLB)
 * 
 * Version 1-0
 * 
 * Copyright (c) 2004 University of New South Wales
 * 
 * All rights reserved. 
 * 
 * Developed by: Operating Systems and Distributed Systems Group (DiSy)
 *               University of New South Wales
 *               http://www.disy.cse.unsw.edu.au
 * 
 * Permission is granted by University of New South Wales, free of charge, to
 * any person obtaining a copy of this software and any associated
 * documentation files (the "Software") to deal with the Software without
 * restriction, including (without limitation) the rights to use, copy,
 * modify, adapt, merge, publish, distribute, communicate to the public,
 * sublicense, and/or sell, lend or rent out copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimers in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the name of University of New South Wales, nor the names of its
 *       contributors, may be used to endorse or promote products derived
 *       from this Software without specific prior written permission.
 * 
 * EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
 * PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS", AND
 * NATIONAL ICT AUSTRALIA AND ITS CONTRIBUTORS MAKE NO REPRESENTATIONS,
 * WARRANTIES OR CONDITIONS OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO ANY REPRESENTATIONS, WARRANTIES OR CONDITIONS
 * REGARDING THE CONTENTS OR ACCURACY OF THE SOFTWARE, OR OF TITLE,
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT,
 * THE ABSENCE OF LATENT OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF
 * ERRORS, WHETHER OR NOT DISCOVERABLE.
 * 
 * TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
 * NATIONAL ICT AUSTRALIA OR ITS CONTRIBUTORS BE LIABLE ON ANY LEGAL
 * THEORY (INCLUDING, WITHOUT LIMITATION, IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY CLAIM, LOSS, DAMAGES OR OTHER
 * LIABILITY, INCLUDING (WITHOUT LIMITATION) LOSS OF PRODUCTION OR
 * OPERATION TIME, LOSS, DAMAGE OR CORRUPTION OF DATA OR RECORDS; OR LOSS
 * OF ANTICIPATED SAVINGS, OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR
 * OTHER ECONOMIC LOSS; OR ANY SPECIAL, INCIDENTAL, INDIRECT,
 * CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES, ARISING OUT OF OR IN
 * CONNECTION WITH THIS LICENCE, THE SOFTWARE OR THE USE OF OR OTHER
 * DEALINGS WITH THE SOFTWARE, EVEN IF NATIONAL ICT AUSTRALIA OR ITS
 * CONTRIBUTORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH CLAIM, LOSS,
 * DAMAGES OR OTHER LIABILITY.
 * 
 * If applicable legislation implies representations, warranties, or
 * conditions, or imposes obligations or liability on University of New South
 * Wales or one of its contributors in respect of the Software that
 * cannot be wholly or partly excluded, restricted or modified, the
 * liability of University of New South Wales or the contributor is limited, to
 * the full extent permitted by the applicable legislation, at its
 * option, to:
 * a.  in the case of goods, any one or more of the following:
 * i.  the replacement of the goods or the supply of equivalent goods;
 * ii.  the repair of the goods;
 * iii. the payment of the cost of replacing the goods or of acquiring
 *  equivalent goods;
 * iv.  the payment of the cost of having the goods repaired; or
 * b.  in the case of services:
 * i.  the supplying of the services again; or
 * ii.  the payment of the cost of having the services supplied again.
 * 
 * The construction, validity and performance of this licence is governed
 * by the laws in force in New South Wales, Australia.
 */
#include <LibELF/elf.h>
#include <string.h>

int elf32_checkFile(struct Elf32_Header *file)
{
	if (file->e_ident[EI_MAG0] != ELFMAG0
	    || file->e_ident[EI_MAG1] != ELFMAG1
	    || file->e_ident[EI_MAG2] != ELFMAG2
	    || file->e_ident[EI_MAG3] != ELFMAG3)
		return -1;	/* not an elf file */
	
	if (file->e_ident[EI_CLASS] != ELFCLASS32)
		return -2;	/* not 32-bit file */
	
	return 0;		/* elf file looks OK */
}

/**
 * Sections Access functions
 */
 
char * elf32_getSectionStringTable(struct Elf32_Header *elfFile)
{
	return (elf32_getStringTableIdx(elfFile) == SHN_UNDEF)
		? NULL
		: (char *)(uintptr_t)elf32_getSectionOffset(elfFile, elf32_getStringTableIdx(elfFile));
}

char * elf32_getSectionName(struct Elf32_Header *elfFile, uint16_t sh)
{
	char *strTable = elf32_getSectionStringTable(elfFile);
	
	return (strTable != NULL)
		? (uintptr_t)elfFile + strTable + (uintptr_t)elf32_getSectionTable(elfFile)[sh].sh_name
		: NULL;
}

void * elf32_getSectionOffsetNamed(struct Elf32_Header *elfFile, char *str)
{
	for (uint16_t i = 0; i < elf32_getNumSections(elfFile); i++)
	{
		if (strcmp(str, elf32_getSectionName(elfFile, i)) == 0)
			return (void *)(uintptr_t)elf32_getSectionOffset(elfFile, i);
	}

	return NULL;
}

int32_t elf32_getSectionIdxNamed(struct Elf32_Header *elfFile, char *str)
{
	for (uint16_t i = 0; i < elf32_getNumSections(elfFile); i++)
	{
		if (strcmp(str, elf32_getSectionName(elfFile, i)) == 0)
			return i;
	}

	return -1;
}

/**
 * Symbols Access functions
 */

void *elf32_getSymbolNamed(struct Elf32_Header *elfFile, char *str)
{
	int symTableIdx = elf32_getSymbolTableIdx(elfFile, ".symtab");

	char *strTable = (symTableIdx != -1) 
		? (char *)((uintptr_t)elfFile + (uintptr_t)elf32_getSymbolStrTable(elfFile, symTableIdx))
		: NULL;

	struct Elf32_Sym *symTable = (symTableIdx != -1) 
		? (struct Elf32_Sym *)((uintptr_t)elfFile + (uintptr_t)elf32_getSectionOffset(elfFile, symTableIdx))
		: NULL;

	if(strTable != NULL && symTable != NULL)
	{
		unsigned int nbSym = elf32_getSectionSize(elfFile, symTableIdx) / elf32_getSectionEntsize(elfFile, symTableIdx);
		
		for(int i = 0; i < nbSym; i++)
		{
			if(strcmp(str, (char *)((uintptr_t)strTable + (uintptr_t)symTable[i].st_name)) == 0)
				return &(symTable[i]);
		}
	}

	return NULL;
}

