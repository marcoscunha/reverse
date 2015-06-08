/*
 * The MAGIC_INFO macro is used to get information about the lock, the spinlock, ...
 * These information is then used by native simulation to synchronized when needed the different simulated core.
 */

#ifndef __MAGIC_COMMON_LOCK_INFO_MACRO_H__
#define __MAGIC_COMMON_LOCK_INFO_MACRO_H__

#ifndef __ASSEMBLER__

typedef struct magic_lock_descr_ {
  unsigned long long pc;
  char name[64 - sizeof(unsigned long long)];
} magic_lock_descr_t;

#endif /* __ASSEMBLER__ */

#if 0 /* should test 32b version */
#define MAGIC_LOCK_INFO(y) \
                { \
                  __asm__("1:\n"\
                          ".pushsection .lock_annotation\n"\
                           ".balign 64\n" \
                           ".long   1b\n"\
                           ".long   0\n"\
                           ".asciz \"" #y "\"\n"\
                           ".popsection\n"\
                         ); \
               }

#define MAGIC_UNLOCK_INFO(y) \
                { \
                  __asm__("1:\n"\
                          ".pushsection .unlock_annotation\n"\
                           ".balign 64\n" \
                           ".long   1b\n"\
                           ".long   0\n"\
                           ".asciz \"" #y "\"\n"\
                           ".popsection\n"\
                         ); \
               }

#define MAGIC_RELEASE_ALL_RECURSIVE_LOCK_INFO(y) \
                { \
                  __asm__("1:\n"\
                          ".pushsection .release_recursive_lock_annotation\n"\
                           ".balign 64\n" \
                           ".long   1b\n"\
                           ".long   0\n"\
                           ".asciz \"" #y "\"\n"\
                           ".popsection\n"\
                         ); \
               }

#else
/* 64b version */
#define MAGIC_LOCK_INFO(y) \
                { \
                  __asm__("1:\n"\
                          ".pushsection .lock_annotation\n"\
                           ".balign 64\n" \
                           ".quad   1b\n"\
                           ".asciz \"" #y "\"\n"\
                           ".popsection\n"\
                         ); \
               }

#define MAGIC_UNLOCK_INFO(y) \
                { \
                  __asm__("1:\n"\
                          ".pushsection .unlock_annotation\n"\
                           ".balign 64\n" \
                           ".quad   1b\n"\
                           ".asciz \"" #y "\"\n"\
                           ".popsection\n"\
                         ); \
               }

#define MAGIC_RELEASE_ALL_RECURSIVE_LOCK_INFO(y) \
                { \
                  __asm__("1:\n"\
                          ".pushsection .release_recursive_lock_annotation\n"\
                           ".balign 64\n" \
                           ".quad   1b\n"\
                           ".asciz \"" #y "\"\n"\
                           ".popsection\n"\
                         ); \
               }

#endif



#endif /* __MAGIC_COMMON_LOCK_INFO_MACRO_H__ */

