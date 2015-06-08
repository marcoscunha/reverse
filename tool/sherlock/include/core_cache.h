#ifndef _CORE_CACHE_H_
#define _CORE_CACHE_H_

#include <cfg.h>
#include <hwetrace.h>
#include <events/hwe_events.h>
#include <events/hwe_device.h>
#include <hwe_handle_def.h>


typedef enum cache_type_t{
    ICACHE,
    DCACHE,
} cache_type_t;

typedef struct cache_t {
    uint8_t      comp_type;       /*Must be the first field in all components */
    hwe_id_dev_t id;
    cache_type_t type;
/*  uint8_t      level:4;
    union{
        cpu_t*          cpu;
        struct cache_t*        cache; // When it is an cache level 2 (minimum)
    }master;*/
    // XXX: FOR NOW IT DOES NOT CARE ABOUT DATA
    //
#ifdef CACHE_METADATA
    CACHE_METADATA
#endif
}cache_t;

void cache_init(event_t *e);
cache_t* cache_create(event_t *e);


#endif // _CORE_CACHE_H_
