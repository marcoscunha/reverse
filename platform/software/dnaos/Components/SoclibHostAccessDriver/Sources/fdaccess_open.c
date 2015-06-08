/*
 * Copyright (C) 2007 TIMA Laboratory                                    
 *                                                                       
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or     
 * (at your option) any later version.                                   
 *                                                                       
 * This program is distributed in the hope that it will be useful,       
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
 * GNU General Public License for more details.                          
 *                                                                       
 * You should have received a copy of the GNU General Public License     
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include <Private/Driver.h>
#include <MemoryManager/MemoryManager.h>
#include <DnaTools/DnaTools.h>

status_t fdaccess_open (char * name, int32_t mode, void ** data)
{
  fdaccess_t fd = NULL;
  int32_t errno = 0, result = 0, device_id = 0;
  char token[DNA_FILENAME_LENGTH], filename[DNA_FILENAME_LENGTH];

  watch (status_t)
  {
    fd = kernel_malloc (sizeof (struct _fdaccess), true);
    ensure (fd != NULL, DNA_OUT_OF_MEM);

    /*
     * Compute the FdAccess device number and its
     * related device ID.
     */

    name += 15;
    dna_strcpy (filename, "fdaccess.");

    path_get_next_entry (& name, token);
    device_id = dna_atoi (token);
    dna_strcat (filename, token);
    dna_strcat (filename, ".");

    path_get_next_entry (& name, token);
    dna_strcat (filename, token);

    dna_log(INFO_LEVEL, "Opening image %s on device %d.", filename, device_id);

    /*
     * Open the image file.
     */

    fd -> port = SOCLIB_FDACCESS_DEVICES[device_id] . port;

    cpu_write(UINT32, (& fd -> port -> FD_ACCESS_BUFFER), filename);
    cpu_write(UINT32, (& fd -> port -> FD_ACCESS_SIZE), 13);
    cpu_write(UINT32, (& fd -> port -> FD_ACCESS_HOW), 0x1);
    cpu_write(UINT32, (& fd -> port -> FD_ACCESS_MODE), 0x1);
    cpu_write(UINT32, (& fd -> port -> FD_ACCESS_OP), FD_ACCESS_OPEN);

    do cpu_read(UINT32, (& fd -> port -> FD_ACCESS_OP), result);
    while (result);

    cpu_read(UINT32, (& fd -> port -> FD_ACCESS_ERRNO), errno);
    cpu_read(UINT32, (& fd -> port -> FD_ACCESS_RETVAL), result);

    check (bad_file, result != -1, DNA_ERROR);

    /*
     * Fill-in the FD structure and return from the function.
     */

    fd -> errno = errno;
    fd -> descriptor = result;

    *data = (void *) fd;
    return DNA_OK;
  }

  rescue (bad_file)
  {
    kernel_free (fd);
    leave;
  }
}

