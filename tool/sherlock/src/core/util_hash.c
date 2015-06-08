#include <Judy.h>
#include <debug.h>
#include <util_hash.h>

#define JUDYERROR_SAMPLE 1 
#ifndef  HASHSIZE 
//#define HASHSIZE (1 << 8)
//#define HASHSIZE (256)
#define HASHSIZE (257)

#endif

Pvoid_t table[HASHSIZE] = { NULL }; // Declare static hash table
/**
 * @brief 
 *
 * @return Pointer to the table or NULL when an error occurs 
 */
/*ht_t *ht_create(void)
{
    uint32_t i;
    for (i = 0; i < HASHSIZE; i++) 
        table[i] = NULL;
    return NULL;
}
*/
/**
 * @brief This function get the item in judy array or create one 
 *        if it does not exist yet.
 *
 * @param key unique key used to search item
 *
 * @return the pointer to item (item_t*)
 */
item_t *ht_get(uint32_t key)
{ 
    item_t **PValue = NULL;

    JLG(PValue, table[key % HASHSIZE], key / HASHSIZE );

    if (PValue == PJERR) 
        ERROR("Fail allocating memory");

    if(PValue == NULL){
        PValue = ht_put(key);
        *PValue = ht_item_init();
        ((item_t*)*PValue)->key = key;
    }
    
    return *PValue;
}

/**
 * @brief
 *
 * @param key 
 *
 * @return pointer to item
 */
item_t **ht_put(uint32_t key)
{
    item_t **PValue = NULL;

    JLI(PValue, table[key % HASHSIZE], key / HASHSIZE );

    if (PValue == PJERR) 
        ERROR("Fail allocating memory");

    return PValue;
}

/**
 * @brief 
 *
 * @param hash_table
 */
/*void ht_free(ht_t *hash_table)
{

}
*/
/**
 * @brief 
 *
 * @return 
 */
item_t *ht_item_init(void)
{
    item_t *item = calloc(1, sizeof(item_t*));
    item->data   = calloc(1, sizeof(sl_addr_t));
    
    return item;
}

/**
 *
 */
void ht_item_del(item_t* item)
{
   free(item->data);
   free(item);
}
