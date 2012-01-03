#include <assert.h>

#include "tour/runtime.h"
#include "tour/hash.h"


#define FULLNESS(cache)                             \
    ((((cache)->size * 75) / 100) <= (cache)->used)
#define EXPANSION(cache)                        \
    ((cache)->size * 2)

#define HASH(_self_, _key_)                     \
    ((*(_self_)->hash_func)( (_self_), _key_ ))
#define COMPARE(_self_, _left_, _right_)            \
    ((*(_self_)->compare_func)( _left_, _right_ ))


cache_ptr tour_hash_new(unsigned int size, hash_func_type hash_func, compare_func_type compare_func)
{
    cache_ptr cache;

    assert( size );
    assert( !(size & (size - 1) ) );

    cache = (cache_ptr)tour_calloc( 1, sizeof(struct cache) );
    assert( cache );

    cache->node_table = (node_ptr *)tour_calloc( size, sizeof(node_ptr) );
    assert( cache->node_table );

    cache->size = size;

    cache->mask = size - 1;

    cache->hash_func = hash_func;

    cache->compare_func = compare_func;

    return cache;
}


void tour_hash_delete(cache_ptr cache)
{
    node_ptr node;
    node_ptr next_node;
    unsigned int i;

    for ( i = 0; i < cache->size; ++ i ) {
        if ( node = cache->node_table[i] ) {
            while ( next_node = node->next ) {
                tour_hash_remove( cache, node->key );
                node = next_node;
            }
            tour_hash_remove( cache, node->key );
        }
    }
    tour_free( cache->node_table );
    tour_free( cache );
}


void tour_hash_add(cache_ptr* cache_p, const void* key, void* value)
{
    /* size_t index = (*(*cache_p)->hash_func)( *cache_p, key ); */
    size_t index = HASH((*cache_p), key);
    node_ptr node = (node_ptr)tour_calloc( 1, sizeof(struct cache_node) );

    assert( node );

    node->key = key;
    node->value = value;
    node->next = (*cache_p)->node_table[index];
#ifdef DEBUG
    { node_ptr node1 = (*cache_p)->node_table[index];
        while ( node1 ) {
            assert( node1->key != key );
            node1 = node1->key;
        }
    }
#endif  /* def DEBUG */

    (*cache_p)->node_table[index] = node;

    ++ (*cache_p)->used;

    if ( FULLNESS(*cache_p) ) {
        node_ptr node1 = NULL;
        cache_ptr new = tour_hash_new( EXPANSION(*cache_p),
                                       (*cache_p)->hash_func,
                                       (*cache_p)->compare_func );

        DEBUG_PRINTF( "Expanding cache %#x from %d to %d\n",
                      (int)*cache_p,
                      *cache_p,
                      (*cache_p)->size,
                      new->size );

        while ( ( node1 = tour_hash_next( *cache_p, node1 ) ) ) {
            tour_hash_add( &new, node1->key, node1->value );
        }

        tour_hash_delete( *cache_p );

        *cache_p = new;
    }
}


void tour_hash_remove(cache_ptr cache, const void* key)
{
    /* size_t index = (*cache->hash_func)( cache, key ); */
    size_t index = HASH(cache, key);
    node_ptr node = cache->node_table[index];

    assert( node );

    if ( (*cache->compare_func)( node->key, key ) ) {
        cache->node_table[index] = node->next;
        tour_free( node );
    } else {
        node_ptr prev = node;
        Boolean removed = FALSE;

        do {
            /* if ( (*cache->compare_func)( node->key, key ) ) { */
            if ( COMPARE( cache, node->key, key ) ) {
                prev->next = node->next, removed = TRUE;
                tour_free( node );
            } else
                prev = node, node = node->next;
        } while ( !removed && node );
        assert( removed );
    }
    -- cache->used;
}


node_ptr tour_hash_next(cache_ptr cache, node_ptr node)
{
    if ( !node )
        cache->last_bucket = 0;

    if ( node ) {
        if ( node->next )
            return node->next;
        else
            ++ cache->last_bucket;
    }
    if ( cache->last_bucket < cache->size ) {
        while ( cache->last_bucket < cache->size ) {
            if ( cache->node_table[cache->last_bucket] )
                return cache->node_table[cache->last_bucket];
            else
                ++ cache->last_bucket;
        }
        return NULL;
    } else
        return NULL;
}


void* tour_hash_value_for_key(cache_ptr cache, const void* key)
{
    node_ptr node = cache->node_table[HASH(cache, key)];
    void* retval = NULL;

    if ( node ) {
        do {
            if ( COMPARE( cache, node->key, key ) ) {
                retval = node->value;
                break;
            } else
                node = node->next;
        } while ( !retval && node );
    }
    return retval;
}


Boolean tour_hash_is_key_in_hash(cache_ptr cache, const void* key)
{
    node_ptr node = cache->node_table[HASH(cache, key)];

    if ( node ) {
        do {
            if ( COMPARE( cache, node->key, key ) ) {
                return TRUE;
            } else
                node = node->next;
        } while ( node );
    }
    return FALSE;
}
