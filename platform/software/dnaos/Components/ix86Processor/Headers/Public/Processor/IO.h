#ifndef PROCESSOR_IO_H
#define PROCESSOR_IO_H

#include <stdint.h>

#define BUILDIO(bwl, bw, type) \
static inline void out##bwl (unsigned type value, int port) \
{ \
    __asm__ volatile ("out" #bwl " %" #bw "0, %w1" \
             : : "a" (value), "Nd" (port)); \
} \
 \
static inline unsigned type in##bwl (int port) \
{ \
    unsigned type value; \
    __asm__ volatile("in" #bwl " %w1, %" #bw "0" \
             : "=a" (value) : "Nd" (port)); \
    return value; \
}

BUILDIO(b, b, char)
BUILDIO(w, w, short)
BUILDIO(l, , int)


#define cpu_write(type,addr,value) cpu_write_##type(addr,value)

#define cpu_write_UINT8(port,value)                                   \
    outb (value, (int) port)

#define cpu_write_UINT16(addr,value)                                  \
    outw (value, (int) port)

#define cpu_write_UINT32(port,value)                                  \
    outl (value,(int) port)

/*
 * Read operations.
 */

#define cpu_read(type,addr,value) cpu_read_##type(addr,value)

#define cpu_read_UINT8(addr,value)                                    \
    (value) = inb (addr)

#define cpu_read_UINT16(addr, value)                                  \
    (value) = inw (addr)

#define cpu_read_UINT32(addr,value)                                   \
    (value) = inl (addr)

/*
 * Uncached operations.
 */

#define cpu_uncached_write(type,addr,value) cpu_write(type,addr,value)
#define cpu_uncached_read(type,addr,value) cpu_read(type,addr,value)

#define cpu_vector_write(mode,to,from,len) cpu_vector_write_##mode(to,from,len)

/*
 * Vector operations
 */

#define cpu_vector_write_DFLOAT(to,from,len)                            \
{                                                                       \
    for (uint32_t i = 0; i < len; i++)                                  \
        ((volatile double *)to)[i] = ((volatile double *)from)[i];      \
}

#define cpu_vector_write_SFLOAT(to,from,len)                            \
{                                                                       \
    for (uint32_t i = 0; i < len; i++)                                  \
        ((volatile float *)to)[i] = ((volatile float *)from)[i];        \
}

#define cpu_vector_write_UINT64(to,from,len)                            \
{                                                                       \
    volatile uint64_t * ulli_to, * ulli_from;                           \
                                                                        \
    ulli_to = (volatile uint64_t *) to;                                 \
    ulli_from = (volatile uint64_t *) from;                             \
                                                                        \
    for (uint32_t i = 0; i < len; i++)                                  \
        ulli_to[i] = ulli_from[i];                                      \
}

#define cpu_vector_write_UINT32(to,from,len)                            \
    cpu_vector_transfer (from, to, len << 2)

#define cpu_vector_write_UINT16(to,from,len)                            \
    cpu_vector_transfer (from, to, len << 1)

#define cpu_vector_write_UINT8(to,from,len)                             \
    cpu_vector_transfer (from, to, len)

#define cpu_vector_read(mode,to,from,len) cpu_vector_write_##mode(to,from,len)

inline uint64_t get_cycles (void)
{
    uint32_t        lo, hi;
    __asm__ __volatile__ (
        // serialize
        "xorl %%eax,%%eax\ncpuid"
        ::: "%rax", "%rbx", "%rcx", "%rdx");

    /* We cannot use "=A", since this would use %rax on x86_64 and return only the lower 32bits of the TSC */
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
 
    return ((uint64_t) hi) << 32 | lo;
}

#endif

