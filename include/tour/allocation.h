#ifndef tourmaline_allocation_h
#define tourmaline_allocation_h


void* tour_malloc(size_t size);


void* tour_atomic_malloc(size_t size);


void* tour_valloc(size_t size);


void* tour_realloc(void * ptr, size_t new_size);


void* tour_calloc(size_t nelem, size_t size);


void tour_free(void* memp);


TOUR_EXPORT void* (*_tour_malloc)(size_t);
TOUR_EXPORT void* (*_tour_atomic_malloc)(size_t);
TOUR_EXPORT void* (*_tour_valloc)(size_t);
TOUR_EXPORT void* (*_tour_realloc)(void*, size_t);
TOUR_EXPORT void* (*_tour_calloc)(size_t, size_t);
TOUR_EXPORT void  (*_tour_free)(void *);


#endif  /* tourmaline_allocation_h */
