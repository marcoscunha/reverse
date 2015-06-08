#ifndef PROCESSOR_IO_H
#define PROCESSOR_IO_H

#define cpu_write(type,addr,value) cpu_write_##type(addr,value)
#define cpu_write_UINT8(addr,value) *((volatile unsigned char *)(addr)) = (value)
#define cpu_write_UINT16(addr,value) *((volatile unsigned short *)(addr)) = (value)
#define cpu_write_UINT32(addr,value) *((volatile unsigned long int *)(addr)) = (value)

#define cpu_read(type,addr,value) cpu_read_##type(addr,value)
#define cpu_read_UINT8(addr,value) (value) = *((volatile unsigned char *) (addr))
#define cpu_read_UINT16(addr, value) (value) = *((volatile unsigned short *) (addr))
#define cpu_read_UINT32(addr,value) (value) = *((volatile unsigned long int *) (addr))


#define cpu_uncached_write(type,addr,value) cpu_write(type,addr,value)
#define cpu_uncached_read(type,addr,value) cpu_read(type,addr,value)

#define cpu_vector_write(mode,to,from,len) cpu_vector_write_##mode(to,from,len)

#define cpu_vector_write_DFLOAT(to,from,len) {    \
  for (unsigned long int i = 0; i < len; i++)      \
    ((volatile double *)to)[_idx] = ((volatile double *)from)[_idx];  \
}

#define cpu_vector_write_SFLOAT(to,from,len) {    \
  for (unsigned long int i = 0; i < len; i++)      \
    ((volatile float *)to)[_idx] = ((volatile float *)from)[_idx];  \
}

#define cpu_vector_write_UINT64(to,from,len) {                                      \
  for (unsigned long int i = 0; i < len; i++)                                        \
    ((volatile unsigned long long int *)to)[_idx] = ((volatile unsigned long long int *)from)[_idx];  \
}

#define cpu_vector_write_UINT32(to,from,len) {                              \
  for (unsigned long int i = 0; i < len; i++)                                \
    ((volatile unsigned long int *)to)[_idx] = ((volatile unsigned long int *)from)[_idx];    \
}

#define cpu_vector_write_UINT16(to,from,len) {                              \
  for (unsigned long int i = 0; i < len; i++)                                \
    ((volatile unsigned short int *)to)[_idx] = ((volatile unsigned long int *)from)[_idx];    \
}

#define cpu_vector_write_UINT8(to,from,len) {                                \
  for (unsigned long int i = 0; i < len; i++)                                \
    ((volatile unsigned char *)to)[_idx] = ((volatile unsigned char *)from)[_idx];            \
}

#define cpu_vector_read(mode,to,from,len) \
  cpu_vector_write_##mode(to,from,len)

#endif

