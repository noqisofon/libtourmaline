#ifndef tourmaline_sarray_h
#define tourmaline_sarray_h

#include "thread.h"

#define   TOURMALINE_SPARSE2
/* #define   TOURMALINE_SPARSE3 */

#ifdef TOURMALINE_SPARSE2
extern const char* __tour_sparse2_id;
#endif  /* def TOURMALINE_SPARSE2 */

#ifdef TOURMALINE_SPARSE3
extern const char* __tour_sparse3_id;
#endif  /* def TOURMALINE_SPARSE2 */

#include <stddef.h>
#include <assert.h>


_EXTERN_C_BEGIN


extern int global_nbuckets;
extern int global_nindeces;
extern int global_narrays;
extern int global_idx_size;


#define SIZET_BITS    (sizeof(size_t) * 8)

#if defined(__sparc__) || defined(TOURMALINE_SPARSE2)
#   define PRECOMPUTE_SELECTORS
#endif  /* defined(__sparc__) || defined(TOURMALINE_SPARSE2) */


#ifndef TOURMALINE_SPARSE3

#   define BUCKET_BITS 3
#   define BUCKET_SIZE (1 << BUCKET_BITS)
#   define BUCKET_MASK (BUCKET_SIZE - 1)

#   define INDEX_BITS  3
#   define INDEX_SIZE  (1 << INDEX_BITS)
#   define INDEX_MASK  (INDEX_SIZE - 1)

#   define INDEX_CAPACITY (BUCKET_SIZE * INDEX_SIZE)

#else  /* def TOURMALINE_SPARSE2 */

#   define BUCKET_BITS 5
#   define BUCKET_SIZE (1 << BUCKET_BITS)
#   define BUCKET_MASK (BUCKET_SIZE - 1)

#endif  /* not def TOURMALINE_SPARSE3 */

typedef size_t sidx;


#ifdef PRECOMPUTE_SELECTORS

struct soffset {
#   ifdef TOURMALINE_SPARSE3
    unsigned int unused : SIZE_BITS / 4 ;
    unsigned int eoffset : SIZE_BITS / 4 ;
    unsigned int boffset : SIZE_BITS / 4 ;
    unsigned int ioffset : SIZE_BITS / 4 ;
#   else
#       ifdef __sparc__
    unsigned long boffset : (SIZET_BITS - 2) - SIZE_BITS;
    unsigned int eoffset : BUCKET_BITS ;
    unsigned int unused : 2 ;
#       else
    unsigned int boffset : SIZET_BITS / 2;
    unsigned int eoffset : SIZET_BITS / 2;
#       endif  /* def __sparc__ */
#   endif  /* def TOURMALINE_SPARSE3 */
};


union sofftype {
    struct soffset off;
    sidx idx;
};

#endif  /* def PRECOMPUTE_SELECTORS */


union sversion {
    int version;
    void* next_tree;
};


struct sbucket {
    void* elems[BUCKET_SIZE];
    union sversion version;
};


#ifdef TOURMALINE_SPARSE3

struct sindex {
    struct sbucket* buckets[INDEX_SIZE];
    union sversion version;
};

#endif  /* def TOURMALINE_SPARSE3 */


struct sarray {
#ifdef TOURMALINE_SPARSE3
    struct sindex* indives;
    struct sindex* empty_index;
#else  /* def TOURMALINE_SPARSE2 */
    struct sbucket** buckets;
#endif  /* def TOURMALINE_SPARSE3 */
    struct sbucket* empty_bucket;
    union sversion version;
    short ref_count;
    struct sarray* is_copy_of;
    size_t capacity;
};


struct sarray* sarray_new(int, void* default_element);


void sarray_free(struct sarray*);


struct sarray* sarray_lazy_copy(struct sarray*);


void sarray_realloc(struct sarray*, int new_size);


void sarray_at_put(struct sarray*, sidx indx, void* elem);


void sarray_at_put_safe(struct sarray*, sidx indx, void* elem);


void sarray_remove_garbage(void);


struct sarray* sarray_hard_copy(struct sarray*);


#ifdef PRECOMPUTE_SELECTORS
static inline unsigned int soffset_decode(sidx indx)
{
    union sofftype x;

    x.idx = indx;
#   ifdef TOURMALINE_SPARSE3
    return x.off.eoffset
        + (x.off.boffset * BUCKET_SIZE)
        + (x.off.ioffset * INDEX_CAPACCITY);
#   else  /* def TOURMALINE_SPARSE2 */
    return x.off.eoffset + (x.off.boffset * BUCKET_SIZE);
#   endif  /* def TOURMALINE_SPARSE3 */
}


static inline sidx soffset_encode(size_t offset)
{
    union sofftype x;

    x.off.eoffset = offset % BUCKET_SIZE;
#ifdef TOURMALINE_SPARSE3
    x.off.boffset = (offset / BUCKET_SIZE) % INDEX_SIZE;
    x.off.ioffset = offset / INDEX_CAPACITY;
#else  /* def TOURMALINE_SPARSE2 */
    x.off.boffset = offset / BUCKET_SIZE;
#endif  /* def TOURMALINE_SPARSE3 */
    return (sidx)x.idx;
}
#else  /* not def PRECOMPUTE_SELECTORS */

static inline size_t soffset_decode(sidx indx)
{
    return indx;
}


static inline sidx soffset_encode(size_t offset)
{
    return offset
}
#endif  /* def PRECOMPUTE_SELECTORS */


static inline void* sarray_get(struct sarray* array, sidx indx)
{
#ifdef PRECOMPUTE_SELECTORS
    union sofftype x;

    x.idx = indx;
#   ifdef TOURMALINE_SPARSE3
    return array->
        indices[x.off.ioffset]->
        buckets[x.off.boffset]->
        elems[x.off.eoffset];
#   else  /* def TOURMALINE_SPARSE2 */
    return array->buckets[x.off.boffset]->elems[x.off.eoffset];
#   endif  /* def TOURMALINE_SPARSE3 */

#else  /* not def PRECOMPUTE_SELECTORS */

#   ifdef TOURMALINE_SPARSE3
    return array->
        indices[indx / INDEX_CAPACITY]->
        buckets[(indx / BUCKET_SIZE) % INDEX_SIZE]->
        elems[indx % BUCKET_SIZE];
#   else  /* def TOURMALINE_SPARSE2 */
    return array->buckets[indx / BUCKET_SIZE]->elems[indx % BUCKET_SIZE];
#   endif  /* def TOURMALINE_SPARSE3 */
#endif  /* def PRECOMPUTE_SELECTORS */
}


static inline void* sarray_get_safe(struct sarray* array, sidx indx)
{
    if ( soffset_decode( indx ) < array->capacity )
        return sarray_get( array, indx );
    else
        return array->empty_bucket->elems[0];
}


_EXTERN_C_END


#endif  /* tourmaline_sarray_h */
