#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "tour/runtime.h"
#include "tour/allocation.h"


#ifdef TOUR_WITH_GC
#   include <gc.h>

static void GC_calloc(size_t nelem, size_t size)
{
    void* p = GC_malloc( nelem * size );

    if ( !p )
        tour_error( nil, TOUR_ERR_MEMORY, "virtual memory exhaustend\n" );
    memset( p, 0, nelem * size );

    return p;
}


static void no_free(void* p)
{
}


TOUR_DECLARE void* (*_tour_malloc)(size_t _0) = GC_malloc;
TOUR_DECLARE void* (*_tour_atomic_malloc)(size_t _0) = GC_atomic_malloc;
TOUR_DECLARE void* (*_tour_valloc)(size_t _0) = GC_malloc;
TOUR_DECLARE void* (*_tour_remalloc)(void * _0, size_t _1) = GC_realloc;
TOUR_DECLARE void* (*_tour_calloc)(size_t _0, size_t _1) = GC_calloc;
TOUR_DECLARE void  (*_tour_free)(void * _0) = no_free;

#else

TOUR_DECLARE void* (*_tour_malloc)(size_t _0) = malloc;
TOUR_DECLARE void* (*_tour_atomic_malloc)(size_t _0) = malloc;
TOUR_DECLARE void* (*_tour_valloc)(size_t _0) = malloc;
TOUR_DECLARE void* (*_tour_realloc)(void * _0, size_t _1) = realloc;
TOUR_DECLARE void* (*_tour_calloc)(size_t _0, size_t _1) = calloc;
TOUR_DECLARE void  (*_tour_free)(void * _0) = free;

#endif  /* def TOUR_WITH_GC */


#define TOUR_MALLOC(_size_)            (*_tour_malloc)( _size_ )
#define TOUR_ATOMIC_MALLOC(_size_)     (*_tour_atomic_malloc)( _size_ )
#define TOUR_VALLOC(_size_)            (*_tour_valloc)( _size_ )
#define TOUR_REALLOC(_ptr_, _size_)    (*_tour_realloc)( _ptr_, _size_ )
#define TOUR_CALLOC(_nelem_, _size_)   (*_tour_calloc)(_nelem_, _size_)
#define TOUR_FREE(_ptr_)               (*_tour_free)(_ptr_)


void* tour_malloc(size_t size)
{
    void* res = TOUR_MALLOC(size);

    if ( !res )
        tour_error( nil, TOUR_ERR_MEMORY, "virtual memory exhaustend\n" );
    return res;
}


void* tour_atomic_malloc(size_t size)
{
    void* res = TOUR_ATOMIC_MALLOC(size);

    if ( !res )
        tour_error( nil, TOUR_ERR_MEMORY, "virtual memory exhaustend\n" );
    return res;
}


void* tour_valloc(size_t size)
{
    void* res = TOUR_VALLOC(size);

    if ( !res )
        tour_error( nil, TOUR_ERR_MEMORY, "virtual memory exhaustend\n" );
    return res;
}


void* tour_realloc(void * ptr, size_t new_size)
{
    void* res = TOUR_REALLOC(ptr, new_size);

    if ( !res )
        tour_error( nil, TOUR_ERR_MEMORY, "virtual memory exhaustend\n" );
    return res;
}


void* tour_calloc(size_t nelem, size_t size)
{
    void* res = TOUR_CALLOC(nelem, size);

    if ( !res )
        tour_error( nil, TOUR_ERR_MEMORY, "virtual memory exhaustend\n" );
    return res;
}


void tour_free(void* memp)
{
    TOUR_FREE(memp);
}
