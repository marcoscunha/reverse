#ifndef CPU_IO_H
#define CPU_IO_H

#define CPU_WRITE(type,addr,value) CPU_WRITE_##type(addr,value)
#define CPU_WRITE_UINT8(addr,value) *((volatile unsigned char *)(addr)) = (value)
#define CPU_WRITE_UINT16(addr,value) *((volatile unsigned short *)(addr)) = (value)
#define CPU_WRITE_UINT32(addr,value) *((volatile unsigned long int *)(addr)) = (value)

#define CPU_READ(type,addr,value) CPU_READ_##type(addr,value)
#define CPU_READ_UINT8(addr,value) (value) = *((volatile unsigned char *) (addr))
#define CPU_READ_UINT16(addr, value) (value) = *((volatile unsigned short *) (addr))
#define CPU_READ_UINT32(addr,value) (value) = *((volatile unsigned long int *) (addr))


#define CPU_UNCACHED_WRITE(type,addr,value) CPU_WRITE(type,addr,value)
#define CPU_UNCACHED_READ(type,addr,value) CPU_READ(type,addr,value)

#define CPU_VECTOR_WRITE(mode,to,from,len) CPU_VECTOR_WRITE_##mode(to,from,len)

#define CPU_VECTOR_WRITE_DFLOAT(to,from,len) {		\
	for (unsigned long int i = 0; i < len; i++)			\
		((volatile double *)to)[_idx] = ((volatile double *)from)[_idx];	\
}

#define CPU_VECTOR_WRITE_SFLOAT(to,from,len) {		\
	for (unsigned long int i = 0; i < len; i++)			\
		((volatile float *)to)[_idx] = ((volatile float *)from)[_idx];	\
}

#define CPU_VECTOR_WRITE_UINT64(to,from,len) {																			\
	for (unsigned long int i = 0; i < len; i++)																				\
		((volatile unsigned long long int *)to)[_idx] = ((volatile unsigned long long int *)from)[_idx];	\
}

#define CPU_VECTOR_WRITE_UINT32(to,from,len) {															\
	for (unsigned long int i = 0; i < len; i++)																\
		((volatile unsigned long int *)to)[_idx] = ((volatile unsigned long int *)from)[_idx];		\
}

#define CPU_VECTOR_WRITE_UINT16(to,from,len) {															\
	for (unsigned long int i = 0; i < len; i++)																\
		((volatile unsigned short int *)to)[_idx] = ((volatile unsigned long int *)from)[_idx];		\
}

#define CPU_VECTOR_WRITE_UINT8(to,from,len) {																\
	for (unsigned long int i = 0; i < len; i++)																\
		((volatile unsigned char *)to)[_idx] = ((volatile unsigned char *)from)[_idx];						\
}

#define CPU_VECTOR_READ(mode,to,from,len) \
	CPU_VECTOR_WRITE_##mode(to,from,len)

#endif

