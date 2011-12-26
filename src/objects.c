#include "tconfig.h"
#include "tour/tour.h"
#include "tour/runtime.h"

#ifdef TOUR_WITH_GC
#   include <gc.h>
#endif  /* def TOUR_WITH_GC */


id __tour_object_alloc(Class);
id __tour_object_dispose(id);
id __tour_object_copy(id);

id (*_tour_object_alloc)(Class) = __tour_object_alloc;
id (*_tour_object_dispose)(id) = __tour_object_dispose;
id (*_tour_object_copy)(id) = __tour_object_copy;


id class_create_instance(Class class)
{
    id new = nil;

#ifdef TOUR_WITH_GC
    if ( CLS_ISCLASS(class) )
        new = (id)GC_malloc_explicitly_typed( class->instance_size,
                                              class->gc_object_type );
#else
    if ( CLS_ISCLASS(class) )
        new = (*_tour_object_alloc)( class );
#endif  /* def TOUR_WITH_GC */

    if ( new != nil ) {
        memset( new, 0, class->instance_size );
        new->class_pointer = class;
    }
    return new;
}


id object_copy(id object)
{
    if ( object != nil && CLS_ISCLASS(object->class_pointer) )
        return (*_tour_object_copy)( object );
    else
        return nil;
}


id object_dispose(id object)
{
    if ( object != nil && CLS_ISCLASS(object->class_pointer) ) {
        if ( _tour_object_dispose )
            (*_tour_object_dipose)( object );
        else
            tour_free( object );
    }
    return nil;
}


id __tour_object_alloc(Class class) {
    return (id)tour_malloc( class->instance_size );
}


id __tour_object_dispose(id object) {
    tour_free( object );

    return nil;
}


id __tour_object_copy(id object) {
    id copy = class_create_instance( object->class_pointer );

    memcpy( copy, object, object->class_pointer->instance_size );

    return copy;
}


// Local Variables:
//   coding: utf-8
// end:
