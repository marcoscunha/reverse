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

int sem_post(sem_t *sem)
{
  status_t status;
  int32_t result = 0;
  semaphore_info_t info;

  watch (int)
  {
    check (invalid_argument, sem != NULL, -1);

    if ((status = semaphore_get_info(sem->id, &info)) != DNA_OK)
      result = EINVAL;
    else if (info.tokens >= SEM_VALUE_MAX)
      result = EOVERFLOW;
    else if ((status = semaphore_release(sem->id, 1, 0)) != DNA_OK)
      result = EINVAL;

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
}
