#ifndef tourmaline_hash_h
#define tourmaline_hash_h

#include <stddef.h>
#include <string.h>

#include "tour.h"


_EXTERN_C_BEGIN


typedef struct cache_node {
    struct cache_node* next;

    const void* key;
    void* value;
} *node_ptr;


typedef unsigned int (*hash_func_type)(void*, const void*);


typedef int (*compare_func_type)(const void*, const void*);


typedef struct cache {
    node_ptr* node_table;

    unsigned int size;
    unsigned int used;
    unsigned int mask;

    unsigned int last_bucket;

    hash_func_type hash_func;
    compare_func_type compare_func;
} *cache_ptr;


extern cache_ptr module_hash_table, class_hash_table;


cache_ptr tour_hash_new( unsigned int size,
                         hash_func_type hash_func,
                         compare_func_type compare_func );


void tour_hash_delete(cache_ptr cache);


void tour_hash_add(cache_ptr* cachep, const void* key, void* value);


void tour_hash_remove(cache_ptr cache, const void* key);


node_ptr tour_hash_next(cache_ptr cache, node_ptr node);


/*!
 * ハッシュテーブルから指定されたキーに対応する値を返します。
 */
void* tour_hash_value_for_key(cache_ptr cache, const void* key);


/*!
 * ハッシュテーブルから指定されたキーに対応する値があるかどうか調べます。
 */
Boolean tour_hash_is_key_in_hash(cache_ptr cache, const void* key);


static inline unsigned int tour_hash_ptr(cache_ptr cache, const void* key)
{
    return ( (size_t)key / sizeof(void *) ) & cache->mask;
}


static inline unsigned int tour_hash_string(cache_ptr cache, const void* key)
{
    unsigned int ret = 0;
    unsigned int ctr = 0;
    const char* ckey = (const char *)key;

    while ( *ckey ) {
        ret ^= *ckey ++ << ctr;
        ctr = (ctr + 1) % sizeof(void *);
    }
    return ret & cache->mask;
}


static inline int tour_compare_ptrs(const void* left_key, const void* right_key)
{
    return left_key == right_key;
}


static inline int tour_compare_strings(const void* left_key, const void* right_key)
{
    if ( left_key == right_key )
        return 1;
    else if ( left_key == NULL || right_key == NULL )
        return 0;
    else
        return !strcmp( (const char *)left_key, (const char *)right_key );
}


_EXTERN_C_END


#endif  /* tourmaline_hash_h */
