#ifndef _UTIL_HASH_H_
#define _UTIL_HASH_H_

#include <sherlock.h>
#include <core_mem.h>

typedef struct {
    uint32_t key;
    sl_addr_t  *data; // Type Fixed
}item_t;

//typedef struct{
//    uint32_t size; 
//    item_t **table;
//}ht_t;

//ht_t     *ht_create(void);
//uint32_t  ht_hash(uint32_t num);
item_t   *ht_get(uint32_t key);
item_t   **ht_put(uint32_t key);
//void      ht_free(ht_t *table);

item_t *ht_item_init(void);

//void ht_item_del(item_t* item);


#endif // _UTIL_HASH_H_
