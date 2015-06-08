#ifndef _CORE_MEM_H_
#define _CORE_MEM_H_

#include <cfg.h>
#include <hwetrace.h>
#include <events/hwe_events.h>
#include <events/hwe_device.h>
#include <hwe_handle_def.h>

#define ALIGN_32BIT       0b11


typedef struct{
    uint32_t value;
#ifdef MEM_METADATA
    MEM_METADATA;
#endif
}sl_addr_t;

typedef struct {
    uint32_t id;
#ifdef MEMDEV_METADATA
    MEMDEV_METADATA;
#endif
}mem_t;

void   mem_init(event_t *e);
mem_t* mem_create(event_t *e);
void   mem_destroy(void);

void  *mem_exec(mem_t *mem, uint32_t addr, uint8_t *data, uint32_t width);

sl_addr_t  *mem_get_addr(uint32_t addr);
uint32_t mem_get_addr_value(uint32_t addr);


#endif // _CORE_MEM_H_
