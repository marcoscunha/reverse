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

status_t rdv_control (void * handler, int32_t operation,
    va_list arguments, int32_t * p_res)
{
  channel_rdv_t * rdv = (channel_rdv_t *)handler;
  bool * result = NULL;
  status_t status = DNA_OK;
  interrupt_status_t it_status;

  it_status = cpu_trap_mask_and_backup ();
  lock_acquire (& rdv -> lock);
  
  switch (operation)
  {
    case RDV_RTEST :
      result = va_arg (arguments, bool *);

      if (rdv -> setter . ready)
      {
        *result = true;
        *p_res = 1;
      }
      else
      {
        *result = false;
        *p_res = 0;
      }

      break;

    case RDV_WTEST :
      result = va_arg (arguments, bool *);

      if (rdv -> getter . ready)
      {
        *result = true;
        *p_res = 1;
      }
      else
      {
        *result = false;
        *p_res = 0;
      }

      break;

    case RDV_RESET :
      if (rdv -> getter . ready || rdv -> setter . ready)
      {
        rdv -> getter . ready = false;
        rdv -> setter . ready = false;
        semaphore_release (rdv -> sem, 1, DNA_NO_RESCHEDULE);
        *p_res = 1;
      }
      else
      {
        *p_res = 0;
      }

      break;

    default :
      *p_res = -1;
      status = DNA_ERROR;
      break;
  }

  lock_release (& rdv -> lock);
  cpu_trap_restore (it_status);
 
  return status;
}

