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

int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
{
  status_t status;
  int32_t result = 0;

  watch (int)
  {
    check (invalid_argument, sem != NULL, -1);
    check (invalid_argument, abs_timeout->tv_nsec >= 0 &&
           abs_timeout->tv_nsec < 1000000000, -1);

    if ((status = semaphore_acquire(sem->id, 1, DNA_ABSOLUTE_TIMEOUT,
        (bigtime_t)(abs_timeout->tv_sec * 1000000000 + abs_timeout->tv_nsec)))
        != DNA_OK)
    {
      switch (status)
      {
        case DNA_TIMED_OUT :
          result = ETIMEDOUT;
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
}
