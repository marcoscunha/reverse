#include <Judy.h>
#include <judy_array.h>

extern "C"
{
    #define HASHSIZE 256
    Pvoid_t table[HASHSIZE] = { NULL }; // Declare static hash table


    uint64_t *judy_get(uint32_t key);
    uint64_t **judy_put(uint32_t key);


    uint64_t *judy_get(uint32_t key)
    {
        uint64_t **PValue = NULL;

        JLG(PValue, table[key % HASHSIZE], key / HASHSIZE );

        if (PValue == PJERR){
            cout << "Fail allocating memory" << endl;
            exit(1);
        }

        if(PValue == NULL){
            PValue = judy_put(key);
            *PValue = malloc(sizeof(uint64_t));
            **PValue = 0x0;
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
    uint64_t **judy_put(uint32_t key)
    {
        uint64_t **PValue = NULL;
    
        JLI(PValue, table[key % HASHSIZE], key / HASHSIZE );
    
        if (PValue == PJERR){
            cout << "Fail allocating memory" << endl;
            exit(1);
        }

        return PValue;
    }
}

