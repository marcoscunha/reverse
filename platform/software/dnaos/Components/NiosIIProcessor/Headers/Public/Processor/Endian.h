#ifndef PROCESSOR_ENDIAN_H
#define PROCESSOR_ENDIAN_H

#include <Platform/Platform.h>

#define SWITCH_ENDIAN_32(x)                                     \
  x =  ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8)  |      \
  ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24)       

#define SWITCH_ENDIAN_16(x) x = ((x & 0x00ff) << 8) |  ((x & 0xff00) >> 8)         

#define cpu_data_is_bigendian(type,data) SWITCH_ENDIAN_##type(data)
#define cpu_data_is_littleendian(type,data)

#if defined(PLATFORM_IS_BIG_ENDIAN)
  #define cpu_platform_to_cpu(type,value) SWITCH_ENDIAN_##type(data)
  #define cpu_cpu_to_platform(type,value) SWITCH_ENDIAN_##type(data)
#elif defined(PLATFORM_IS_LITTLE_ENDIAN)
  #define cpu_platform_to_cpu(type,value)
  #define cpu_cpu_to_platform(type,value)
#else
  #error "ERROR : Platform endianness not set."
#endif

#define cpu_data_concat(size,result,lvalue,hvalue) {    \
  result = (hvalue << size) | lvalue;                    \
}

#define cpu_data_split(size,value,lvalue,hvalue) {       \
  hvalue = value >> size;                                \
  lvalue = value - (hvalue << size);                    \
}

#endif

