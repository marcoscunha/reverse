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
#include <DnaTools/DnaTools.h>

status_t fdaccess_read (void * handler, void * destination,
    int64_t offset, int32_t * p_count)
{
  fdaccess_t fd = (fdaccess_t) handler;
  volatile int32_t result = 0, errno = 0;
  interrupt_status_t it_status = 0;

  watch (status_t)
  {
    ensure (fd -> descriptor != -1, DNA_ERROR);

    it_status = cpu_trap_mask_and_backup ();
    lock_acquire (& fd -> lock);

    /*
     * First, apply a seek to set the host file
     * pointer on the right position.
     */

    cpu_write(UINT32, & (fd -> port -> FD_ACCESS_FD), fd -> descriptor);
    cpu_write(UINT32, & (fd -> port -> FD_ACCESS_SIZE), offset);
    cpu_write(UINT32, & (fd -> port -> FD_ACCESS_MODE), 0); 
    cpu_write(UINT32, & (fd -> port -> FD_ACCESS_OP), FD_ACCESS_LSEEK);

    do cpu_read (UINT32, (& fd -> port -> FD_ACCESS_OP), result);
    while (result);

    cpu_read(UINT32, (& fd -> port -> FD_ACCESS_ERRNO), errno);
    cpu_read(UINT32, (& fd -> port -> FD_ACCESS_RETVAL), result);

    /*
     * Then, execute the read operation.
     */

    if (result != -1)
    {
      cpu_write(UINT32, & (fd -> port -> FD_ACCESS_FD), fd -> descriptor);
      cpu_write(UINT32, & (fd -> port -> FD_ACCESS_BUFFER), destination);
      cpu_write(UINT32, & (fd -> port -> FD_ACCESS_SIZE), *p_count); 
      cpu_write(UINT32, & (fd -> port -> FD_ACCESS_OP), FD_ACCESS_READ);

      do cpu_read (UINT32, (& fd -> port -> FD_ACCESS_OP), result);
      while (result);

      cpu_read(UINT32, (& fd -> port -> FD_ACCESS_ERRNO), errno);
      cpu_read(UINT32, (& fd -> port -> FD_ACCESS_RETVAL), result);
    }

    fd -> errno = errno;

    lock_release (& fd -> lock);
    cpu_trap_restore (it_status);

    *p_count = result;
    return DNA_OK;
  }
}

