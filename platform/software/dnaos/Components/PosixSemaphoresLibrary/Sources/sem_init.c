/*
 * Copyright (C) 2011 TIMA Laboratory. Authors: H. Chen and F. Pétrot
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

#include <Private/PosixSemaphores.h>

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
  status_t status;
  int32_t result = 0;

  watch (int)
  {
    check (invalid_argument, sem != NULL, -1);
    check (invalid_argument, value <= (unsigned int)SEM_VALUE_MAX, -1);
    check (not_supported, pshared == 0, -1);

    /* Posix says that this function should not be called more than once on
       a given semaphore, otherwise the behavior is undefined.
       So we do not take any precautions here.
    */
    if ((status = semaphore_create("", value, &sem->id)) != DNA_OK)
    {
      switch (status)
      {
        case DNA_NO_MORE_SEM :
          result = EAGAIN;
        case DNA_OUT_OF_MEM :
          result = ENOMEM;
        default :
          result = EINVAL;
      }
    }

    if (result != 0)
    {
      errno = result;
      return -1;
    }

    return 0;
  }

  rescue (invalid_argument)
  {
    errno = EINVAL;
    leave;
  }

  rescue (not_supported)
  {
    errno = ENOSYS;
    leave;
  }
}
