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
#include <stdio.h>

/*
 * Checks that elfFile points to a valid elf file. Returns 0 if the elf
 * file is valid, < 0 if invalid. 
 */

#define ISELF32(elfFile) ( ((struct Elf32_Header*)elfFile)->e_ident[EI_CLASS] == ELFCLASS32 )
#define ISELF64(elfFile) ( ((struct Elf64_Header*)elfFile)->e_ident[EI_CLASS] == ELFCLASS64 )

int elf_checkFile(void *elfFile)
{
	return ISELF32 (elfFile)
		? elf32_checkFile(elfFile)
		: elf64_checkFile(elfFile);
}

bool elf_getMemoryBounds(void *elfFile, bool phys, uint64_t *min, uint64_t *max)
{
	uint64_t mem_min = UINT64_MAX;
	uint64_t mem_max = 0;
	int i;

	if (elf_checkFile(elfFile) != 0) {
		return false;
	}

	for(i=0; i < elf_getNumProgramHeaders(elfFile); i++) {
		uint64_t sect_min, sect_max;

		if (elf_getProgramHeaderMemorySize(elfFile, i) == 0) {
			continue;
		}

		if (phys) {
			sect_min = elf_getProgramHeaderPaddr(elfFile, i);
		} else {
			sect_min = elf_getProgramHeaderVaddr(elfFile, i);
		}

		sect_max = sect_min + elf_getProgramHeaderMemorySize(elfFile, i);

		if (sect_max > mem_max) {
			mem_max = sect_max;
		}
		if (sect_min < mem_min) {
			mem_min = sect_min;
		}
	}
	*min = mem_min;
	*max = mem_max;

	return true;
}

uint64_t elf_getEntryPoint(void *elfFile)
{
	return ISELF32 (elfFile)
		? elf32_getEntryPoint (elfFile)
		: elf64_getEntryPoint (elfFile);
}

/**
 * Program Headers Access functions
 */

uint32_t elf_getNumProgramHeaders(void *elfFile)
{
	return ISELF32 (elfFile)
		? elf32_getNumProgramHeaders(elfFile)
		: elf64_getNumProgramHeaders(elfFile);
}

uint64_t elf_getProgramHeaderPaddr(void *elfFile, uint32_t ph)
{
	return ISELF32 (elfFile)
		? elf32_getProgramHeaderPaddr(elfFile, ph)
		: elf64_getProgramHeaderPaddr(elfFile, ph);
}

uint64_t elf_getProgramHeaderVaddr(void *elfFile, uint32_t ph)
{
	return ISELF32 (elfFile)
		? elf32_getProgramHeaderVaddr(elfFile, ph)
		: elf64_getProgramHeaderVaddr(elfFile, ph);
}

uint64_t elf_getProgramHeaderMemorySize(void *elfFile, uint32_t ph)
{
	return ISELF32 (elfFile)
		? elf32_getProgramHeaderMemorySize(elfFile, ph)
		: elf64_getProgramHeaderMemorySize(elfFile, ph);
}

uint64_t elf_getProgramHeaderFileSize(void *elfFile, uint32_t ph)
{
	return ISELF32 (elfFile)
		? elf32_getProgramHeaderFileSize(elfFile, ph)
		: elf64_getProgramHeaderFileSize(elfFile, ph);
}

uint64_t elf_getProgramHeaderOffset(void *elfFile, uint32_t ph)
{
	return ISELF32 (elfFile)
		? elf32_getProgramHeaderOffset(elfFile, ph)
		: elf64_getProgramHeaderOffset(elfFile, ph);
}

uint32_t elf_getProgramHeaderFlags(void *elfFile, uint32_t ph)
{
	return ISELF32 (elfFile)
		? elf32_getProgramHeaderFlags(elfFile, ph)
		: elf64_getProgramHeaderFlags(elfFile, ph);
}

uint32_t elf_getProgramHeaderType(void *elfFile, uint32_t ph)
{
	return ISELF32 (elfFile)
		? elf32_getProgramHeaderType(elfFile, ph)
		: elf64_getProgramHeaderType(elfFile, ph);
}

uint64_t elf_vtopProgramHeader(void *elfFile, uint32_t ph, uint64_t vaddr)
{
	uint64_t ph_phys = elf_getProgramHeaderPaddr(elfFile, ph);
	uint64_t ph_virt = elf_getProgramHeaderVaddr(elfFile, ph);
	uint64_t paddr;

	paddr = vaddr - ph_virt + ph_phys;

	return paddr;
}

bool elf_vaddrInProgramHeader(void *elfFile, uint32_t ph, uint64_t vaddr)
{
	uint64_t min = elf_getProgramHeaderVaddr(elfFile, ph);
	uint64_t max = min + elf_getProgramHeaderMemorySize(elfFile, ph);
	
	return (vaddr >= min && vaddr < max) ? true : false;
}

/**
 * Sections Access functions
 */

uint32_t elf_getNumSections(void *elfFile)
{
	return ISELF32 (elfFile)
		? elf32_getNumSections(elfFile)
		: elf64_getNumSections(elfFile);
}

uint64_t elf_getSectionOffset(void *elfFile, uint32_t sh)
{
	return ISELF32 (elfFile)
		? elf32_getSectionOffset(elfFile, sh)
		: elf64_getSectionOffset(elfFile, sh);
}

uint64_t elf_getSectionSize(void *elfFile, uint32_t sh)
{
	return ISELF32 (elfFile)
		? elf32_getSectionSize(elfFile, sh)
		: elf64_getSectionSize(elfFile, sh);
}

uint64_t elf_getSectionEntsize(void *elfFile, uint32_t sh)
{
	return ISELF32 (elfFile)
		? elf32_getSectionEntsize(elfFile, sh)
		: elf64_getSectionEntsize(elfFile, sh);
}

uint64_t elf_getSectionAddr(void *elfFile, uint32_t sh)
{
	return ISELF32 (elfFile)
		? elf32_getSectionAddr(elfFile, sh)
		: elf64_getSectionAddr(elfFile, sh);
}

uint64_t elf_getSectionFlags(void *elfFile, uint32_t sh)
{
	return ISELF32 (elfFile)
		? elf32_getSectionFlags(elfFile, sh)
		: elf64_getSectionFlags(elfFile, sh);
}

uint64_t elf_getSectionType(void *elfFile, uint32_t sh)
{
	return ISELF32 (elfFile)
		? elf32_getSectionType(elfFile, sh)
		: elf64_getSectionType(elfFile, sh);
}

char * elf_getSectionStringTable(void *elfFile)
{
	return ISELF32 (elfFile)
		? elf32_getSectionStringTable(elfFile)
		: elf64_getSectionStringTable(elfFile);
}

char * elf_getSectionName(void *elfFile, uint32_t sh)
{
	return ISELF32 (elfFile)
		? elf32_getSectionName(elfFile, sh)
		: elf64_getSectionName(elfFile, sh);
}

void * elf_getSectionOffsetNamed(void *elfFile, char *str)
{
	return ISELF32 (elfFile)
		? elf32_getSectionOffsetNamed(elfFile, str)
		: elf64_getSectionOffsetNamed(elfFile, str);
}

int64_t elf_getSectionIdxNamed(void *elfFile, char *str)
{
	return ISELF32 (elfFile)
		? elf32_getSectionIdxNamed(elfFile, str)
		: elf64_getSectionIdxNamed(elfFile, str);
}

/**
 * Symbols Access functions
 */

int64_t elf_getSymbolTableIdx(void *elfFile, char *str)
{
	return ISELF32 (elfFile)
		? elf32_getSymbolTableIdx (elfFile, str)
		: elf64_getSymbolTableIdx (elfFile, str);
}

char *elf_getSymbolStrTable(void *elfFile, uint32_t sh)
{
	return ISELF32 (elfFile)
		? elf32_getSymbolStrTable (elfFile, sh)
		: elf64_getSymbolStrTable (elfFile, sh);
}

uint64_t elf_getSymbolValue(void *elfFile, void *symEntry)
{
	return ISELF32 (elfFile)
		? elf32_getSymbolValue (symEntry)
		: elf64_getSymbolValue (symEntry);
}

void * elf_getSymbolNamed(void *elfFile, char *str)
{
	return ISELF32 (elfFile)
		? elf32_getSymbolNamed (elfFile, str)
		: elf64_getSymbolNamed (elfFile, str);
}

