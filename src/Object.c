#include "config.h"

#include <string.h>

#ifdef TOUR_WITH_GC
#   include <gc.h>
#endif  /* def TOUR_WITH_GC */

#include "tour/tour-api.h"
#include "tour/Class.h"
#include "tour/Object.h"


#define OBJ_ALLOC(_object_)    (*_tour_object_allocate)( _object_ )
#define OBJ_DEALLOC(_object_)  (*_tour_object_deallocate)( _object_ )
#define OBJ_COPY(_object_)     (*_tour_object_copy)( _object_ )


id __tour_object_alloc(Class_ref);
id __tour_object_dealloc(id);
id __tour_object_copy(id);

Class_ref Object = NULL;

id (*_tour_object_allocate)(Class_ref) = __tour_object_alloc;
id (*_tour_object_deallocate)(id)      = __tour_object_dealloc;
id (*_tour_object_copy)(id)            = __tour_object_copy;


id Object_alloc(Class_ref _class)
{
    id new = nil;

    if ( CLASS_IS_CLASS(_class) ) {
#ifdef TOUR_WITH_GC
        new  = (id)GC_malloc_explicitly_typed( CLASS_INSTANCE_SIZE(_class),
                                               CLASS_GC_OBJECT_TYPE(_class) );
#else
        new = OBJ_ALLOC( _class );
#endif  /* def TOUR_WITH_GC */
    }

    if ( new != nil ) {
        memset( new, 0, CLASS_INSTANCE_SIZE(_class) );
        new->class_pointer = _class;
    }
    return new;
}


id Object_init(id object)
{
    return object;
}


id Object_new()
{
    return Object_init( Object_alloc( tour_get_class( "Object" ) ) );
}


id Object_copy(id object)
{
    if ( (object != nil) && CLASS_IS_CLASS(CLASS_CLASSOF(object)) )
        return OBJ_COPY(object);
    else
        return nil;
}


id Object_free(id object)
{
    if ( (object != nil) && CLASS_IS_CLASS(CLASS_CLASSOF(object)) )
        OBJ_DEALLOC( object );
    else
        tour_free( object );
    return nil;
}


id __tour_object_alloc(Class_ref _class)
{
    return (id)tour_malloc( CLASS_INSTANCE_SIZE(_class) );
}


id __tour_object_dealloc(id object)
{
    tour_free( object );

    return nil;
}


id __tour_object_copy(id object)
{
    id copy = Object_alloc( CLASS_CLASSOF(object) );

    memcpy( copy, object, CLASS_INSTANCE_SIZE(CLASS_CLASSOF(object)) );

    return copy;
}
