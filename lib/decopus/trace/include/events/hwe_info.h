#ifndef _HWE_INFO_H_
#define _HWE_INFO_H_

#include "hwe_common.h"
#include "hwe_device.h"

/*
 * Information on device
 *
 * mapping:
 *    HEADER;
 *    hwe_info_t  body;
 *    char        name[body.nsize];
 *    hwe_type>_t detail;
 */

#define HWE_INFO_NAME_MAX 255
typedef struct hwe_info_t {
   uint16_t      version;  // version used
   hwe_device_t  device:8; // type of the device
   uint8_t       nsize;    // size of the name (excluding '\0' chararcter)
} __attribute__((__packed__)) hwe_info_t;

typedef struct hwe_info_cont {
   hwe_head_cont common;
   hwe_info_t    body;
   char          name[HWE_INFO_NAME_MAX + 1]; // name
   hwe_devices_u detail;
} hwe_info_cont;


#endif // _HWE_INFO_H_


/**********************************************************************
 ********     TOOLS     ***********************************************
 **********************************************************************/

#ifdef HWE_USE_TOOLS
#ifndef _HWE_TOOLS_INFO_H_
#define _HWE_TOOLS_INFO_H_

#include "hwe_device.h"

/*
 * compute the size needed to store an info event
 */
static inline size_t hwe_info_sizeof(const hwe_info_cont *cont)
{
   size_t res = hwe_head_sizeof(&cont->common);
   res += sizeof(hwe_info_t) + cont->body.nsize;
   res += hwe_device_sizeof(cont->body.device);
   return res;
}

/*
 * write the info event to the given memory location
 * which must have enough allocated space
 * return the address following the event
 */
static inline void * hwe_info_write(const hwe_info_cont *cont, void *dest)
{
   size_t nbytes = sizeof(hwe_info_t);
   memcpy(dest, &cont->body, nbytes);
   dest += nbytes;
   
   nbytes = cont->body.nsize;
   memcpy(dest, &cont->name, nbytes);
   dest += nbytes;
  
   nbytes = hwe_device_sizeof(cont->body.device); 
   memcpy(dest, &cont->detail, nbytes);
   return dest + nbytes;
}

/*
 * read
 */
static inline size_t hwe_info_read(hwe_info_cont *cont,
      const void *buf, const size_t size)
{
   size_t cur, need;
   
   cur = sizeof(hwe_info_t);
   need  = cur;
   if (size < need) return 0;
   memcpy(&cont->body, buf, cur);
   buf += cur;

   if (cont->body.version != HWE_VERSION) {
      fprintf(stderr, 
              "Bad version number in HWE_INFO: "
              "looking for %u.%u, found %u.%u.\n",
              HWE_VERSION_MAJOR, HWE_VERSION_MINOR,
              cont->body.version >> 8, cont->body.version & 0xff);
      exit(1);
   }

   cur = cont->body.nsize;
   need += cur;
   if (size < need) return 0;
   memcpy(&cont->name, buf, cur);
   cont->name[cur] = '\0';
   buf += cur;
  
   cur = hwe_device_sizeof(cont->body.device); 
   need += cur;
   if (size < need) return 0;
   memcpy(&cont->detail, buf, cur);
   
   return need;
}

/*
 * print
 */
static inline void hwe_info_print(FILE *stream, const hwe_info_cont *cont)
{
   if (cont->body.nsize != 0)
      fprintf(stream, "\tName: `%s'\n", cont->name);
   hwe_device_print(stream, "\t", cont->body.device, &cont->detail);
}

/*
 * desc
 */
static inline int hwe_info_desc(const hwe_info_cont *cont, char *str, int len)
{
	if (cont->body.nsize != 0)
   	return snprintf(str, len, " `%s'", cont->name);
	else
		return 0;
}

#endif // _HWE_TOOLS_INFO_H_
#endif // HWE_USE_TOOLS

