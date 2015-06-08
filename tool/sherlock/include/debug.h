#ifndef  _DEBUG_H_
#define _DEBUG_H_

/*#include <stdio.h>
#include <stdarg.h>

#define ERROR(str) printf("ERROR:[%s:%d]: %s: " str"\n",__FILE__,__LINE__, __FUNCTION__,##__VA_ARGS__);*/
#define ERROR(str) printf("ERROR:[%s:%d]: %s: " str"\n",__FILE__,__LINE__, __FUNCTION__);


#define PEVENT(event) printf("[%d.%d]\n",(uint32_t)((hwe_cont*)event)->common.id.devid, \
                                       (uint32_t)((hwe_cont*)event)->common.id.index);


#endif
