#ifndef __DEFAULT_INLINE_H__
#define __DEFAULT_INLINE_H__

#include "default_struct.h"
#include "magic_lock_info_macro.h"

static __inline__ void
__initlock(__lock_struct_t *lock)
{
	lock->lock = __UNLOCKED;
}

static __inline__ void
__lock(__lock_struct_t *lock, int id)
{
  MAGIC_LOCK_INFO(lock);
	__lock_struct_t old_value, new_value;
	do {
		old_value.lock = __UNLOCKED;
		new_value.lock = id+1;
	} while (__sync_val_compare_and_swap(&(lock->lock), old_value.lock, new_value.lock) != old_value.lock);

}

static __inline__ void
__unlock(__lock_struct_t *lock, int id)
{
	__lock_struct_t old_value, new_value;
	do {
		old_value.lock = id+1;
		new_value.lock = __UNLOCKED;
	} while (__sync_val_compare_and_swap(&(lock->lock), old_value.lock, new_value.lock) != old_value.lock);
  MAGIC_UNLOCK_INFO(lock);
}

#endif /* __DEFAULT_INLINE_H__ */

