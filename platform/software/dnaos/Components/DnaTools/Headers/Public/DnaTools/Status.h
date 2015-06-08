/****h* Tools/Status
 * SUMMARY
 * Status definitions.
 ****
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
 ****/

#ifndef DNA_TOOLS_STATUS_H
#define DNA_TOOLS_STATUS_H

#include <stdint.h>

/****t* Status/status_t
 * SUMMARY
 * Status type
 *
 * SOURCE
 */
#ifdef WRITEBACK
typedef uint32_t status_t;
  /*
   * This code are not errors, hence they are
   * given positive numbers
   */

#define  DNA_HANDLED_INTERRUPT              0x0005
#define  DNA_UNHANDLED_INTERRUPT            0x0004
#define  DNA_INVOKE_SCHEDULER               0x0003
#define  DNA_ALREADY_AT_ROOT                0x0002
#define  DNA_NO_ERROR                       0x0001
#define  DNA_OK                             0x0000

  /*
   * Standard error codes
   */

#define  DNA_ERROR                          0xFFFF
#define  DNA_NOT_IMPLEMENTED                0xFFFE
#define  DNA_OUT_OF_MEM                     0xFFFD
#define  DNA_BAD_ARGUMENT                   0xFFFC
#define  DNA_ACCES_DENIED                   0xFFFB
#define  DNA_NAME_TOO_LONG                  0xFFFA
#define  DNA_INTERRUPTED                    0xFFF9

  /*
   * VFS error codes
   */

#define  DNA_NO_ENTRY                       0xFEFF
#define  DNA_NO_VOLUME                      0xFEFE
#define  DNA_NO_VNODE                       0xFEFD
#define  DNA_DEVICE_MOUNTED                 0xFEFC
#define  DNA_VNODE_MOUNTED                  0xFEFB
#define  DNA_UNKNOWN_FILESYSTEM             0xFEFA
#define  DNA_MAX_OPENED_FILES               0xFEF9
#define  DNA_INVALID_FD                     0xFEF8
#define  DNA_BAD_INODE_TYPE                 0xFEF7
#define  DNA_INVALID_WHENCE                 0xFEF6
#define  DNA_VOLUME_IN_USE                  0xFEF5
#define  DNA_INODE_EXISTS                   0xFEF4

  /*
   * Semaphore error codes
   */

#define  DNA_BAD_SEM_ID                     0xFDFF
#define  DNA_NO_MORE_SEM                    0xFDFE
#define  DNA_WOULD_BLOCK                    0xFDFD
#define  DNA_TIMED_OUT                      0xFDFC
#define  DNA_SEM_DESTROYED                  0xFDFB

  /*
   * Threads error codes
   */

#define  DNA_UNKNOWN_THREAD                 0xFCFF
#define  DNA_INVALID_THREAD_ID              0xFCFE
#define  DNA_NO_AVAILABLE_THREAD            0xFCFD
#define  DNA_NO_MORE_THREAD                 0xFCFC
#define  DNA_THREAD_DESTROYED               0xFCFB

  /*
   * Timer error codes
   */

#define  DNA_NO_TIMER                       0xFBFF
#define  DNA_UNKNOWN_ALARM                  0xFBFE
#define  DNA_NO_MORE_ALARM                  0xFBFD

  /*
   * Ports and messages codes
   */

#define  DNA_BAD_PORT_ID                    0xFAFF
#define  DNA_NO_MORE_PORT                   0xFAFE
#define  DNA_UNKNOWN_PORT                   0xFAFD

#else
typedef enum _status
{
  /*
   * This code are not errors, hence they are
   * given positive numbers
   */

  DNA_HANDLED_INTERRUPT               = 0x0005,
  DNA_UNHANDLED_INTERRUPT             = 0x0004,
  DNA_INVOKE_SCHEDULER                = 0x0003,
  DNA_ALREADY_AT_ROOT                 = 0x0002,
  DNA_NO_ERROR                        = 0x0001,
  DNA_OK                              = 0x0000,

  /*
   * Standard error codes
   */

  DNA_ERROR                           = 0xFFFF,
  DNA_NOT_IMPLEMENTED                 = 0xFFFE,
  DNA_OUT_OF_MEM                      = 0xFFFD,
  DNA_BAD_ARGUMENT                    = 0xFFFC,
  DNA_ACCES_DENIED                    = 0xFFFB,
  DNA_NAME_TOO_LONG                   = 0xFFFA,
  DNA_INTERRUPTED                     = 0xFFF9,

  /*
   * VFS error codes
   */

  DNA_NO_ENTRY                        = 0xFEFF,
  DNA_NO_VOLUME                       = 0xFEFE,
  DNA_NO_VNODE                        = 0xFEFD,
  DNA_DEVICE_MOUNTED                  = 0xFEFC,
  DNA_VNODE_MOUNTED                   = 0xFEFB,
  DNA_UNKNOWN_FILESYSTEM              = 0xFEFA,
  DNA_MAX_OPENED_FILES                = 0xFEF9,
  DNA_INVALID_FD                      = 0xFEF8,
  DNA_BAD_INODE_TYPE                  = 0xFEF7,
  DNA_INVALID_WHENCE                  = 0xFEF6,
  DNA_VOLUME_IN_USE                   = 0xFEF5,
  DNA_INODE_EXISTS                    = 0xFEF4,

  /*
   * Semaphore error codes
   */

  DNA_BAD_SEM_ID                      = 0xFDFF,
  DNA_NO_MORE_SEM                     = 0xFDFE,
  DNA_WOULD_BLOCK                     = 0xFDFD,
  DNA_TIMED_OUT                       = 0xFDFC,
  DNA_SEM_DESTROYED                   = 0xFDFB,

  /*
   * Threads error codes
   */

  DNA_UNKNOWN_THREAD                  = 0xFCFF,
  DNA_INVALID_THREAD_ID               = 0xFCFE,
  DNA_NO_AVAILABLE_THREAD             = 0xFCFD,
  DNA_NO_MORE_THREAD                  = 0xFCFC,
  DNA_THREAD_DESTROYED                = 0xFCFB,

  /*
   * Timer error codes
   */

  DNA_NO_TIMER                        = 0xFBFF,
  DNA_UNKNOWN_ALARM                   = 0xFBFE,
  DNA_NO_MORE_ALARM                   = 0xFBFD,

  /*
   * Ports and messages codes
   */

  DNA_BAD_PORT_ID                     = 0xFAFF,
  DNA_NO_MORE_PORT                    = 0xFAFE,
  DNA_UNKNOWN_PORT                    = 0xFAFD
}
status_t;
#endif
/*
 ****/

#endif
