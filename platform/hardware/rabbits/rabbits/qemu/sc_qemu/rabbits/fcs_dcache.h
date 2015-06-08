#ifndef _FCS_DCACHE_H_
#define _FCS_DCACHE_H_

#include "rabbits/cfg.h"

#ifdef RABBITS_IMPLEMENT_CACHES

#if DATA_SIZE == 8
    #define DATA_SIZE_BITS 64
#elif DATA_SIZE == 4
    #define DATA_SIZE_BITS 32
#elif DATA_SIZE == 2
    #define DATA_SIZE_BITS 16
#elif DATA_SIZE == 1
    #define DATA_SIZE_BITS 8
#endif
extern void dcache_invalidate_all(void);
extern void dcache_invalidate(unsigned long addr);
extern void dcache_flush_all(void);
extern void dcache_flush(unsigned long addr);

extern void * REGPARM dcache_read_direct (unsigned long addr, uint32_t size);
extern void * REGPARM dcache_read (unsigned long addr);
extern unsigned long long dcache_read_q (unsigned long addr);
extern unsigned long dcache_read_l (unsigned long addr);
extern unsigned short dcache_read_w (unsigned long addr);
extern unsigned char dcache_read_b (unsigned long addr);
extern signed short dcache_read_signed_w (unsigned long addr);
extern signed char dcache_read_signed_b (unsigned long addr);

extern void REGPARM dcache_write (unsigned long addr, int nb, unsigned long val);
extern void dcache_write_q (unsigned long addr, unsigned long long val);
#define dcache_write_l(addr,val) dcache_write (addr, 4, val)
#define dcache_write_w(addr,val) dcache_write (addr, 2, val)
#define dcache_write_b(addr,val) dcache_write (addr, 1, val)

#define tswap8(v) (v)

#endif //RABBITS_IMPLEMENT_CACHES

#endif //_FCS_DCACHE_H_
