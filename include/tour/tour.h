#ifndef tourmaline_tour_h
#define tourmaline_tour_h

#include <assert.h>

#include "GenericTypes.h"


struct tour_object {
    Class_ref class_pointer;
};

#define OBJ_CLASSOF(_object_)     ((_object_)->class_pointer)

typedef struct tour_object* id;


typedef const struct tour_selector {
    void* sel_id;
    const char* sel_types;
} *SEL;


inline static Boolean sel_eq(SEL left, SEL right)
{
    if ( left == NULL || right == NULL )
        return left == right;
    else
        return left->sel_id == right->sel_id;
}


typedef id (*IMP)(id _0, SEL _1, ...);


struct tour_class {
    MetaClass_ref               class_pointer;

    Class_ref                   super_class;

    const char*                 name;
    int                         version;
    unsigned int                info;

    unsigned int                instance_size;

    struct tour_ivar_list*      ivars;

    struct tour_method_list*    methods;

    struct sarray*              dtable;

    struct tour_class*          subclass_list;
    struct tour_class*          sibling_class;

    void*                       gc_object_type;
};


#define nil (id)0
#define Nil (Class_ref)0


typedef void* retval_t;
typedef void (*apply_t)(void);

typedef union arglist {
    char* arg_ptr;
    char arg_regs[sizeof(char*)];
} *arglist_t;


Boolean tour_init(int* argc, char*** argv);
void tour_quit();


#endif  /* tourmaline_tour_h */
