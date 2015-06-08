#ifndef __DEFAULT_STRUCT_H__
#define __DEFAULT_STRUCT_H__

#include <stdint.h>

#define __UNLOCKED 0
#define __LOCKED   1

typedef union __lock_struct {
	uint32_t lock;
} __lock_struct_t;


#endif /* __DEFAULT_STRUCT_H__ */

