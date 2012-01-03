#ifndef tourmaline_Class_h
#define tourmaline_Class_h

#include "tour.h"
#include "messaging.h"


_EXTERN_C_BEGIN


#define _C_ID           '@'
#define _C_CLASS        '#'
#define _C_SEL          ':'
#define _C_CHR          'c'
#define _C_UCHR         'C'
#define _C_SHT          's'
#define _C_USHT         'S'
#define _C_INT          'i'
#define _C_UINT         'I'
#define _C_LNG          'l'
#define _C_ULNG         'L'
#define _C_LNG_LNG      'q'
#define _C_ULNG_LNG     'Q'
#define _C_FLT          'f'
#define _C_DBL          'd'
#define _C_BFLD         'b'
#define _C_BOOL         'B'
#define _C_VOID         'v'
#define _C_UNDEF        '?'
#define _C_PTR          '^'
#define _C_CHARPTR      '*'
#define _C_ATOM         '%'
#define _C_ARY_B        '['
#define _C_ARY_E        ']'
#define _C_UNION_B      '('
#define _C_UNION_E      ')'
#define _C_STRUCT_B     '{'
#define _C_STRUCT_E     '}'
#define _C_VECTOR       '!'
#define _C_COMPLEX      'j'


typedef struct tour_superclass {
    id self;

#ifdef __cplusplus
    Class_ref super_class;
#else
    Class_ref class;
#endif  /* def __cplusplus */
} Super, *Super_ref;


#define  CLASS_CLASSOF(_class_)          OBJ_CLASSOF(_class_)
#define  CLASS_SUPER_CLASSOF(_class_)    ((_class_)->super_class)
#define  CLASS_GET_CLASSNAME(_class_)    ((_class_)->name)
#define  CLASS_INFO(_class_)             ((_class_)->info)
#define  CLASS_INSTANCE_SIZE(_class_)    ((_class_)->instance_size)
#define  CLASS_GC_OBJECT_TYPE(_class_)   ((_class_)->gc_object_type)


#define  __CLASS_IS_INFO(_class_, _mask_)    ((CLASS_INFO(_class_) & _mask_) == _mask_)
#define  __CLASS_SETINFO(_class_, _mask_)    (CLASS_INFO(_class_) |= _mask_)

#define  _CLASSID_META             0x01L
#define  CLASS_IS_META(_class_)              __CLASS_IS_INFO(_class_, _CLASSID_META)

#define  _CLASSID_CLASS            0x02L
#define  CLASS_IS_CLASS(_class_)             __CLASS_IS_INFO(_class_, _CLASSID_CLASS)

#define  _CLASSID_INITIALIZED      0x04L
#define  CLASS_IS_INITIALIZED(_class_)       __CLASS_IS_INFO(_class_, _CLASSID_INITIALIZED)
#define  CLASS_SETINITIALIZED(_class_)       __CLASS_SETINFO(_class_, _CLASSID_INITIALIZED)

#define  _CLASSID_RESOLV           0x08L
#define  CLASS_IS_RESOLV(_class_)            __CLASS_IS_INFO(_class_, _CLASSID_RESOLV)
#define  CLASS_SETRESOLV(_class_)            __CLASS_SETINFO(_class_, _CLASSID_RESOLV)

#define  CLASS_GETNUMBER(_class_)            (CLASS_INFO(_class_) >> (HOST_BITS_PER_LONG / 2))
#define  CLASS_SETNUMBER(_class_, _num_)                                \
    ({ (_class_)->info <<= (HOST_BITS_PER_LONG / 2);                    \
        (_class_)->info >>= (HOST_BITS_PER_LONG / 2);                   \
        __CLASS_SETINFO(_class_, (((unsigned long)_num_) << (HOST_BITS_PER_LONG / 2))); })


Method_ref Class_get_class_method(MetaClass_ref _class, SEL op);


Method_ref Class_get_instance_method(Class_ref _class, SEL op);


Class_ref Class_pose_as(Class_ref impostor, Class_ref superclass);


Class_ref tour_get_class(const char* name);


Class_ref tour_lookup_class(const char* name);


Class_ref tour_next_class(void** enume_state);


extern id Class_create_instance(Class_ref _class);


static inline const char* Class_get_class_name(Class_ref _class)
{
    return CLASS_IS_CLASS(_class) ? CLASS_GET_CLASSNAME(_class) : ( (_class == Nil) ? "Nil": NULL );
}


static inline int Class_get_instance_size(Class_ref _class)
{
    return CLASS_IS_CLASS(_class) ? _class->instance_size: 0;
}


static inline MetaClass_ref Class_get_meta_class(Class_ref _class)
{
    return CLASS_IS_CLASS(_class) ? _class->class_pointer: Nil;
}


static inline Class_ref Class_get_super_class(Class_ref _class)
{
    return CLASS_IS_CLASS(_class) ? _class->super_class: Nil;
}


static inline int Class_get_version(Class_ref _class)
{
    return CLASS_IS_CLASS(_class) ? _class->version: -1;
}


static inline Boolean Class_is_class(Class_ref _class)
{
    return CLASS_IS_CLASS(_class);
}


static inline Boolean Class_is_metaclass(Class_ref _class)
{
    return CLASS_IS_META(_class);
}


static inline void Class_set_version(Class_ref _class, int version)
{
    if ( CLASS_IS_CLASS(_class) )
        _class->version = version;
}


static inline void* Class_get_gc_object_type(Class_ref _class)
{
    return CLASS_IS_CLASS(_class) ? _class->gc_object_type: NULL;
}


_EXTERN_C_END


#endif  /* tourmaline_Class_h */
