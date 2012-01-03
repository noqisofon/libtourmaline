#ifndef test_TOUR_Object_h
#define test_TOUR_Object_h

#include "tour.h"
#include "GenericTypes.h"


extern Class_ref Object;


id Object_alloc(Class_ref _class);


id Object_init(id object);


id Object_new();


#ifdef __NeXT__
#   define Object_copy gnu_object_copy
#   define Object_free gnu_object_free
#endif  /* def __NeXT__ */


id Object_copy(id object);


id Object_free(id object);


static inline Class_ref Object_get_class(id object)
{
    return ( (object != nil) ? ( CLASS_IS_CLASS(OBJ_CLASSOF(object))
                                 ? OBJ_CLASSOF(object)
                                 : ( CLASS_IS_META(OBJ_CLASSOF(object))
                                     ? (Class_ref)object
                                     : Nil ) )
             : Nil );
}


static inline const char* Object_get_class_name(id object)
{
    return ( (object != nil) ? ( CLASS_IS_CLASS(OBJ_CLASSOF(object))
                                 ? CLASS_GET_CLASSNAME(OBJ_CLASSOF(object))
                                 : ((Class_ref)object)->name )
             : "Nil" );
}


static inline MetaClass_ref Object_get_meta_class(id object)
{
    return ( (object != nil) ? ( CLASS_IS_CLASS(OBJ_CLASSOF(object))
                                 ? CLASS_CLASSOF(OBJ_CLASSOF(object))
                                 : (CLASS_IS_META(OBJ_CLASSOF(object))
                                    ? OBJ_CLASSOF(object)
                                    : Nil) )
             : Nil );
}


static inline Class_ref Object_get_super_class(id object)
{
    return ( (object != nil) ? ( CLASS_IS_CLASS(OBJ_CLASSOF(object))
                                 ? CLASS_SUPER_CLASSOF(OBJ_CLASSOF(object))
                                 : ( CLASS_IS_META(OBJ_CLASSOF(object))
                                     ? CLASS_SUPER_CLASSOF((Class_ref)object)
                                     : Nil ) )
             : Nil );
}


static inline Boolean Object_is_class(id object)
{
    return ( (object != nil) && CLASS_IS_META(OBJ_CLASSOF(object)) );
}


static inline Boolean Object_is_instance(id object)
{
    return ( (object != nil) && CLASS_IS_CLASS(OBJ_CLASSOF(object)) );
}


static inline Boolean Object_is_meta_class(id object)
{
    return ( (object != nil)
             && !Object_is_instance( object )
             && !Object_is_class( object ) );
}


struct sarray* tour_get_uninstalled_dtable(void);


#endif  /* test_TOUR_Object_h */
