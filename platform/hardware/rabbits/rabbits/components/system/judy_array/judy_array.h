#ifndef JUDY_ARRAY_H
#define JUDY_ARRAY_H

extern "C"
{
    uint64_t *judy_get(uint32_t key);
    uint64_t **judy_put(uint32_t key);
}

#endif
