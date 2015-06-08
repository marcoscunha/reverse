#ifndef _HWE_DEVICE_H_
#define _HWE_DEVICE_H_

#include <stdint.h>

typedef enum hwe_device_t {
   HWE_PROCESSOR,
   HWE_MEMORY,
   HWE_CACHE,
   HWE_WRITEBUFFER,
   HWE_PERIPHERAL,
} hwe_device_t;

/*
 * processor
 */
typedef struct hwe_processor_t {
   uint8_t cpuid;
} __attribute__((__packed__)) hwe_processor_t;

/*
 * cache
 */
typedef struct hwe_cache_t {
   uint16_t numset;// number -1 of sets
   uint16_t numline;// number -1 of lines per set
   uint16_t numbyte;// number -1 of bytes per line
} __attribute__((__packed__)) hwe_cache_t;


/*
 * memory
 */
typedef struct hwe_memory_t {
   uint32_t baseaddr;
   uint32_t endaddr;
   unsigned cached:1;
} __attribute__((__packed__)) hwe_memory_t;

/*
 * peripheral
 */
typedef struct hwe_peripheral_t {
} __attribute__((__packed__)) hwe_peripheral_t;


typedef union hwe_devices_u {
   hwe_processor_t  processor;
   hwe_cache_t      cache;
   hwe_memory_t     memory;
   hwe_peripheral_t peripheral;
} hwe_devices_u;
#endif // _HWERACE_DEVICE_H_

#ifdef HWE_USE_TOOLS
#ifndef _HWE_DEVICE_TOOLS_H_
#define _HWE_DEVICE_TOOLS_H_

static inline size_t hwe_device_sizeof(hwe_device_t dev)
{
   size_t res;
   switch (dev) {
      case HWE_PROCESSOR:
         res = sizeof(hwe_processor_t);
         break;
      case HWE_MEMORY:
         res = sizeof(hwe_memory_t);
         break;
      case HWE_CACHE:
         res = sizeof(hwe_cache_t);
         break;
      case HWE_WRITEBUFFER:
         res = 0;
         break;
      case HWE_PERIPHERAL:
        res = 0;
        break;
      default:
         fprintf(stderr, "Unknown device\n");
         exit(1);
         break;
   }
   return res;
}

/*
 * print
 */
static inline void hwe_device_print(FILE *stream, const char *prefix, 
      hwe_device_t type, const hwe_devices_u *detail)
{
   switch(type) {
      case HWE_PROCESSOR:
         {
            fprintf(stream, "%sProcessor: cpuid=%u\n", prefix,
                  detail->processor.cpuid);
         }
         break;
      case HWE_CACHE:
         fprintf(stream, "%sCache: %u sets * %u lines * %u bytes\n", prefix,
               1U + detail->cache.numset,
               1U + detail->cache.numline,
               1U + detail->cache.numbyte);
         break;
      case HWE_MEMORY:
         fprintf(stream, "%sMemory: [0x%x - 0x%x] %s\n", prefix,
                  detail->memory.baseaddr,
                  detail->memory.endaddr,
                  (detail->memory.cached)?"cached":"uncached");
         break;
      case HWE_WRITEBUFFER:
         {
            fprintf(stream, "%sWrite-Buffer\n", prefix);
         }
         break;
      default:
         fprintf(stream, "%sUnknown\n", prefix);
         break;
   }
}
#endif
#endif



