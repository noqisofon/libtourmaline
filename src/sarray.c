#include "config.h"

#include <stdio.h>
#include <assert.h>

#include "tour/sarray.h"
#include "tour/runtime.h"


int global_nbuckets = 0;
int global_nindeces = 0;
int global_narrays = 0;
int global_idx_size = 0;

static void* first_free_data = NULL;

#ifdef TOUR_SPARSE2
const char* __tour_sparse2_id = "2 level sparse indices";
#endif  /* def TOUR_SPARSE2 */

#ifdef TOUR_SPARSE3
const char* __tour_sparse3_id = "3 level sparse indices";
#endif  /* def TOUR_SPARSE3 */


static void sarray_free_garbage(void* vp)
{
    tour_mutex_lock( __tour_runtime_mutex );

    if ( __tour_runtime_threads_alive == 1 ) {
        tour_free( vp );
        if ( first_free_data )
            sarray_remove_garbage();
    } else {
        *(void **)vp = first_free_data;
        first_free_data = vp;
    }

    tour_mutex_unlock( __tour_runtime_mutex );
}


void sarray_remove_garbage(void)
{
    void** vp;
    void** np;

    tour_mutex_lock( __tour_runtime_mutex );

    vp = first_free_data;
    first_free_data = NULL;

    while ( vp ) {
        np = *vp;
        tour_free( vp );
        vp = np;
    }

    tour_mutex_unlock( __tour_runtime_mutex );
}


void sarray_at_put(struct sarray* array, sidx index, void* element)
{
#ifdef TOUR_SPARSE3
    struct sindex** the_index;
    struct sindex*  new_index;
#endif  /* def TOUR_SPARSE3 */
    struct sbucket** the_bucket;
    struct sbucket*  new_bucket;
#ifdef TOUR_SPARSE3
    size_t ioffset;
#endif  /* def TOUR_SPARSE3 */
    size_t boffset;
    size_t eoffset;
#ifdef PRECOMPUTE_SELECTORS
    union sofftype xx;
    xx.idx = index;
#   ifdef TOUR_SPARSE3
    ioffset = xx.off.ioffset;
#   endif  /* def TOUR_SPARSE3 */
    boffset = xx.off.boffset;
    eoffset = xx.off.eoffset;
#else
#   ifdef TOUR_SPARSE3
    ioffset = index / INDEX_CAPACITY;
    boffset = (index / BUCKET_SIZE) & INDEX_SIZE;
    eoffset = index % BUCKET_SIZE;
#else
    boffset = index / BUCKET_SIZE;
    eoffset = index % BUCKET_SIZE;
#   endif  /* def TOUR_SPARSE3 */
#endif  /* def PRECOMPUTE_SELECTORS */

    assert( soffset_decode(index) < array->capacity );

#ifdef TOUR_SPARSE3
    the_index = &(array->indices[ioffset]);
    the_bucket = &((*the_index)->buckets[boffset]);
#else
    the_bucket = &(array->buckets[boffset]);
#endif  /* def TOUR_SPARSE3 */

    if ( (*the_bucket)->elems[eoffset] == element )
        return ;

#ifdef TOUR_SPARSE3

    if ( (*the_bucket) == array->empty_bucket ) {

        new_bucket = (struct sbucket *)tour_malloc( sizeof(struct sbucket) );
        memcpy( (void *)new_bucket,
                (const void *)array->empty_bucket,
                sizeof(struct bucket) );
        new_bucket->version.version = array->version.version;
        *the_bucket = new_bucket;

        ++ nindices;

    } else if ( (*the_bucket)->version.version != array->version.version ) {

        struct sbucket* old_bucket = *the_bucket;
        new_bucket = (struct sbucket *)tour_malloc( sizeof(struct sbucket) );
        memcpy( new_bucket,
                old_bucket,
                sizeof(struct sbucket) );
        new_bucket->version.version = array->version.version;
        *the_bucket = new_bucket;

        ++ nindices;
    }
    
#endif  /* def TOUR_SPARSE3 */

    if ( (*the_bucket) == array->empty_bucket ) {

        new_bucket = (struct sbucket *)tour_malloc( sizeof(struct sbucket) );
        memcpy( (void *)new_bucket,
                (const void *)array->empty_bucket,
                sizeof(struct sbucket) );
        new_bucket->version.version = array->version.version;
        *the_bucket = new_bucket;

        ++ global_nbuckets;

    } else if ( (*the_bucket)->version.version != array->version.version ) {

        struct sbucket* old_bucket = *the_bucket;
        new_bucket = (struct sbucket *)tour_malloc( sizeof(struct sbucket) );
        memcpy( new_bucket,
                old_bucket,
                sizeof(struct sbucket) );
        new_bucket->version.version = array->version.version;
        *the_bucket = new_bucket;

        ++ global_nbuckets;

    }
    (*the_bucket)->elems[eoffset] = element;
}


void sarray_at_put_safe(struct sarray* array, sidx index, void* element)
{
    size_t soffset = soffset_decode( index );

    if ( soffset >= array->capacity )
        sarray_realloc( array, soffset + 1 );
    sarray_at_put( array, index, element );
}


struct sarray* sarray_new(int size, void* default_element)
{
    struct sarray* array;
#ifdef TOUR_SPARSE3
    size_t num_indices = ( (size - 1) / (INDEX_CAPACITY) ) + 1;
    struct sindex** new_indices;
#else
    size_t num_indices = ( (size - 1) / (BUCKET_SIZE) ) + 1;
    struct sbucket** new_buckets;
#endif  /* def TOUR_SPARSE3 */
    size_t i;

    assert( size > 0 );

    array = (struct sarray *)tour_malloc( sizeof(struct sarray) );
    array->version.version = 0;

#ifdef TOUR_SPARSE3
    array->capacity = num_indices * (INDEX_CAPACITY);
    new_indices = (struct sindex **)tour_malloc( sizeof(struct sindex *) * num_indices );

    array->empty_index = (struct sindex *)tour_malloc( sizeof(struct sindex) );
    array->empty_index->version.version = 0;

    ++ global_narrays;
    global_idx_size += num_indices;
    ++ nindies;

#else
    array->capacity = num_indices * (BUCKET_SIZE);
    new_buckets = (struct sbucket **)tour_malloc( sizeof(struct sbucket *) * num_indices );

    ++ global_narrays;
    global_idx_size += num_indices;
#endif  /* def TOUR_SPARSE3 */

    array->empty_bucket = (struct sbucket *)tour_malloc( sizeof(struct sbucket) );
    array->empty_bucket->version.version = 0;

    ++ global_nbuckets;

    array->ref_count = 1;
    array->is_copy_of = NULL;

    for ( i = 0; i < BUCKET_SIZE; ++ i ) {
        array->empty_bucket->elems[i] = default_element;
    }

#ifdef TOUR_SPARSE3
    for ( i = 0; i < INDEX_SIZE; ++ i ) {
        array->empty_index->buckets[i] = array->empty_bucket;
    }

    for ( i = 0; i < num_indices; ++ i ) {
        new_indices[i] = array->empty_index;
    }
#else
    for ( i = 0; i < num_indices; ++ i ) {
        new_buckets[i] = array->empty_bucket;
    }
#endif  /* def TOUR_SPARSE3 */

#ifdef TOUR_SPARSE3
    array->indices = new_indices;
#else
    array->buckets = new_buckets;
#endif  /* def TOUR_SPARSE3 */

    return array;
}


void sarray_realloc(struct sarray* array, int newsize)
{
#ifdef TOUR_SPARSE3
    size_t old_max_index = (array->capacity - 1) / INDEX_CAPACITY;
    size_t new_max_index = ( (newsize - 1) / INDEX_CAPACITY );
    size_t rounded_size = (new_max_index + 1) * INDEX_CAPACITY;

    struct sindex** new_indices;
    struct sindex** old_indices;
#else
    size_t old_max_index = (array->capacity - 1) / BUCKET_SIZE;
    size_t new_max_index = ( (newsize - 1) / BUCKET_SIZE );
    size_t rounded_size = (new_max_index + 1) * BUCKET_SIZE;

    struct sbucket** new_buckets;
    struct sbucket** old_buckets;
#endif  /* def TOUR_SPARSE3 */
    size_t i;

    assert( newsize > 0 );

    if ( rounded_size <= array->capacity )
        return ;

    assert( array->ref_count == 1 );

    if ( rounded_size > array->capacity ) {
#ifdef TOUR_SPARSE3
        new_max_index += 4;
        rounded_size = (new_max_index + 1) * INDEX_CAPACITY;
#else
        new_max_index += 4;
        rounded_size = (new_max_index + 1) * BUCKET_SIZE;
#endif  /* def TOUR_SPARSE3 */

        array->capacity = rounded_size;

#ifdef TOUR_SPARSE3
        old_indices = array->indices;
        new_indices = (struct sindex **)tour_malloc( (new_max_index + 1) * sizeof(struct sindex *) );
#else
        old_buckets = array->buckets;
        new_buckets = (struct sbucket **)tour_malloc( (new_max_index + 1) * sizeof(struct sbucket *) );
#endif  /* def TOUR_SPARSE3 */

        for ( i = 0; i <= old_max_index; ++ i ) {
#ifdef TOUR_SPARSE3
            new_indices[i] = old_indices[i];
#else
            new_buckets[i] = old_buckets[i];
#endif  /* def TOUR_SPARSE3 */
        }

        for ( i = old_max_index + 1; i <= new_max_index; ++ i ) {
#ifdef TOUR_SPARSE3
            new_indices[i] = array->empty_index;
#else
            new_buckets[i] = array->empty_bucket;
#endif  /* def TOUR_SPARSE3 */
        }

#ifdef TOUR_SPARSE3
        array->indices = new_indices;
#else
        array->buckets = new_buckets;
#endif  /* def TOUR_SPARSE3 */

#ifdef TOUR_SPARSE3
        sarray_free_garbage( old_indices );
#else
        sarray_free_garbage( old_buckets );
#endif  /* def TOUR_SPARSE3 */

        global_idx_size += (new_max_index - old_max_index);

        return ; // なくてもよくね？
    }
}


void sarray_free(struct sarray* array)
{
#ifdef TOUR_SPARSE3
    size_t old_max_index = (array->capacity - 1) / INDEX_CAPACITY;
    struct sindex** old_indices;
#else
    size_t old_max_index = (array->capacity - 1) / BUCKET_SIZE;
    struct sbucket** old_buckets;
#endif  /* def TOUR_SPARSE3 */
    size_t i = 0;

    assert( array->ref_count != 0 );

    if ( --(array->ref_count) != 0 )
        return ;

#ifdef TOUR_SPARSE3
    old_indices = array->indices;
#else
    old_buckets = array->buckets;
#endif  /* def TOUR_SPARSE3 */

    for ( i = 0; i <= old_max_index; ++ i ) {
#ifdef TOUR_SPARSE3
        struct sindex* idx = old_indices[i];

        if ( (idx != array->empty_index)
             && ( idx->version.version == array->version.version ) ) {
            int j;

            for ( j = 0; j < INDEX_SIZE; ++ j ) {
                struct sbucket* bkt = idx->buckets[j];

                if ( (bkt != array->empty_bucket)
                     && ( bkt->version.version == array->version.version ) ) {
                    sarray_free_garbage( bkt );
                    -- global_nbuckets;
                }
            }
            sarray_free_garbage( idx );
            -- nindices;
        }
#else
        struct sbucket* bkt = old_buckets[i];

        if ( (bkt != array->empty_bucket)
             && ( bkt->version.version == array->version.version ) ) {
            sarray_free_garbage( bkt );
            -- global_nbuckets;
        }
#endif  /* def TOUR_SPARSE3 */
    }

#ifdef TOUR_SPARSE3
    if ( array->empty_index->version.version == array->version.version ) {
        sarray_free_garbage( array->empty_index );
        -- nindices;
    }
#endif  /* def TOUR_SPARSE3 */

    if ( array->empty_bucket->version.version == array->version.version ) {
        sarray_free_garbage( array->empty_bucket );
        -- global_nbuckets;
    }
    global_idx_size -= (old_max_index + 1);
    -- global_narrays;

#ifdef TOUR_SPARSE3
    sarray_free_garbage( array->indices );
#else
    sarray_free_garbage( array->buckets );
#endif  /* def TOUR_SPARSE3 */

    if ( array->is_copy_of )
        sarray_free( array->is_copy_of );

    sarray_free_garbage( array );
}


struct sarray* sarray_lazy_copy(struct sarray* other_array)
{
    struct sarray* array;

#ifdef TOUR_SPARSE3
    size_t num_indices = ( (other_array->capacity - 1) / INDEX_CAPACITY ) + 1;
    struct sindex** new_indices;
#else
    size_t num_indices = ( (other_array->capacity - 1) / BUCKET_SIZE ) + 1;
    struct sbucket** new_buckets;
#endif  /* def TOUR_SPARSE3 */

    array = (struct sarray *)tour_malloc( sizeof(struct sarray) );
    array->version.version = other_array->version.version + 1;
#ifdef TOUR_SPARSE3
    array->empty_index = other_array->empty_index;
#endif  /* def TOUR_SPARSE3 */
    array->empty_bucket = other_array->empty_bucket;
    array->ref_count = 1;

    ++ other_array->ref_count;

    array->is_copy_of = other_array;
    array->capacity = other_array->capacity;

#ifdef TOUR_SPARSE3
    new_indices = (struct sindex **)tour_malloc( sizeof(struct sindex *) * num_indices );
    memcpy( new_indices,
            other_array->indices,
            sizeof(struct sindex *) * num_indices );
    array->indices = new_indices;
#else
    new_buckets = (struct sbucket **)tour_malloc( sizeof(struct sbucket *) * num_indices );
    memcpy( new_buckets,
            other_array->buckets,
            sizeof(struct sbucket *) * num_indices );
    array->buckets = new_buckets;
#endif  /* def TOUR_SPARSE3 */

    global_idx_size += num_indices;
    ++ global_narrays;

    return array;
}
