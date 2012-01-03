#include "config.h"

#include "tour/runtime.h"
#include "tour/sarray.h"
#include "tour/encoding.h"
#include "tour/runtime-info.h"
#include "tour/messaging.h"

#include "tour/Class.h"

#define gen_rtx(args...)  1
#define get_rtx_MEM(args...)  1
#define get_rtx_REG(args...)  1

#define rtx int

#if !defined(STRUCT_VALUE) || STRUCT_VALUE == 0
#   define INVISIBLE_STRUCT_RETURN    1
#else
#   define INVISIBLE_STRUCT_RETURN    0
#endif  /* !defined(STRUCT_VALUE) || STRUCT_VALUE == 0 */

#define  __TOUR_RUNTIME_LOCK             tour_mutex_lock( __tour_runtime_mutex )
#define  __TOUR_RUNTIME_UNLOCK           tour_mutex_unlock( __tour_runtime_mutex )

struct sarray* __tour_uninstalled_dtable = NULL;

IMP (*__tour_msg_forward)(SEL _0) = NULL;
IMP (*__tour_msg_forward2)(id _0, SEL _1) = NULL;

static void __tour_send_initialize(Class_ref);

static void __tour_install_dtable_for_class(Class_ref _class);
static void __tour_prepare_dtable_for_class(Class_ref _class);
static void __tour_install_prepared_dtable_for_class(Class_ref _class);


static struct sarray* __tour_prepared_dtable_for_class(Class_ref _class);
static IMP __tour_get_prepared_imp(Class_ref _class, SEL sel);


static double __tour_double_forward(id _0, SEL _1, ...);
static id __tour_word_forward(id _0, SEL _1, ...);

typedef struct { id many[8]; } __big;
#if INVISIBLE_STRUCT_RETURN
static __big __tour_block_forward(id _0, SEL _1, ...);
#else
static id __tour_block_forward(id _0, SEL _1, ...);
#endif  /* INVISIBLE_STRUCT_RETURN */


static Method_ref search_for_method_in_hierarchy(Class_ref _class, SEL sel);


Method_ref search_for_method_in_list(MethodList_ref list, SEL op);


id nil_method(id receiver, SEL op __attribute__ ((__unused__)))
{
    return receiver;
}


inline IMP __tour_get_forward_imp(id receiver, SEL selector)
{
    if ( __tour_msg_forward2 ) {
        IMP result;

        if ( ( result = __tour_msg_forward2( receiver, selector ) ) != NULL )
            return result;
    }

    if ( __tour_msg_forward ) {
        IMP result;

        if ( ( result = __tour_msg_forward( selector ) ) != NULL )
            return result;
    }

    {
        const char* t = selector->sel_types;

        if ( t && (*t == '[' || *t == '(' || *t == '{' )
#ifdef CORE_TOURLNUT_MAX_STRUCT_BY_VALUE
             && tour_sizeof_type( t ) > CORE_TOURLNUT_MAX_STRUCT_BY_VALUE
#endif  /* def CORE_TOURLNUT_MAX_STRUCT_BY_VALUE */
        )
            return (IMP)__tour_block_forward;
        else if ( *t == 'f' || *t == 'd' )
            return (IMP)__tour_double_forward;
        else
            return (IMP)__tour_word_forward;
    }
}


static inline IMP get_implementation(id receiver, Class_ref _class, SEL selector)
{
    void* res;

    if ( _class->dtable == __tour_uninstalled_dtable ) {

        __TOUR_RUNTIME_LOCK;

        if ( _class->dtable == __tour_uninstalled_dtable )
            __tour_install_dtable_for_class( _class );

        if ( _class->dtable == __tour_uninstalled_dtable ) {
            assert( __tour_prepared_dtable_for_class( _class ) != 0 );
            res = __tour_get_prepared_imp( _class, selector );
        } else
            res = NULL;
            
        __TOUR_RUNTIME_UNLOCK;

        if ( !res )
            res = get_implementation( receiver, _class, selector );
    } else {
        res = sarray_get_safe( _class->dtable, (size_t)selector->sel_id );

        if ( res == NULL )
            res = __tour_get_forward_imp( receiver, selector );
    }
    return res;
}


inline IMP get_imp(Class_ref _class, SEL selector)
{
    void* res = sarray_get_safe( _class->dtable, (size_t)selector->sel_id );

    if ( res == NULL )
        res = (void *)get_implementation( nil, _class, selector );
    return res;
}


inline Boolean __tour_responds_to(id object, SEL selector)
{
    void* res;
    struct sarray* dtable;

    dtable = OBJ_CLASSOF(object)->dtable;
    if ( dtable == __tour_uninstalled_dtable ) {

        __TOUR_RUNTIME_LOCK;

        if ( OBJ_CLASSOF(object)->dtable == __tour_uninstalled_dtable )
            __tour_install_dtable_for_class( CLASS_CLASSOF(object) );

        if ( OBJ_CLASSOF(object)->dtable == __tour_uninstalled_dtable ) {
            dtable = __tour_prepared_dtable_for_class( CLASS_CLASSOF(object) );

            assert( dtable );
        } else
            dtable = CLASS_CLASSOF(object)->dtable;

        __TOUR_RUNTIME_UNLOCK;

    }
    res = sarray_get_safe( dtable, (size_t)selector->sel_id );

    return res != 0;
}


inline IMP tour_msg_lookup(id receiver, SEL op)
{
    IMP result;

    if ( receiver ) {
        result = sarray_get_safe( OBJ_CLASSOF(receiver)->dtable, (sidx)op->sel_id );

        if ( result == NULL )
            result = get_implementation( receiver, OBJ_CLASSOF(receiver), op );
        return result;
    } else
        return (IMP)nil_method;
}


IMP tour_msg_lookup_super(Super_ref super, SEL selector)
{
    if ( super->self )
        return get_imp( super->class, selector );
    else
        return (IMP)nil_method;
}


int method_get_sizeof_arguments(Method_ref _0);


retval_t tour_msg_sendv(id object, SEL op, arglist_t arg_frame)
{
    Method_ref m = Class_get_instance_method( OBJ_CLASSOF(object), op );
    const char* type;

    *((id *) method_get_first_argument( m, arg_frame, &type ) ) = object;
    *((SEL *) method_get_next_argument( arg_frame, &type ) ) = op;

    return __builtin_apply( (apply_t)(m->method_imp),
                            arg_frame,
                            method_get_sizeof_arguments( m ) );
}


void __tour_init_dispatch_tables()
{
    __tour_uninstalled_dtable = sarray_new( 200, 0 );
}


void __tour_install_premature_dtable(Class_ref _class)
{
    assert( __tour_uninstalled_dtable );

    _class->dtable = __tour_uninstalled_dtable;
}


static void __tour_send_initialize(Class_ref _class)
{
    assert( CLASS_IS_CLASS(_class) );
    assert( !CLASS_IS_META(_class) );

    if ( !CLASS_IS_INITIALIZED(_class) ) {
        CLASS_SETINITIALIZED(_class);
        CLASS_SETINITIALIZED(CLASS_CLASSOF(_class));

        __tour_generate_gc_type_description( _class );

        if ( CLASS_SUPER_CLASSOF(_class) )
            __tour_send_initialize( CLASS_SUPER_CLASSOF(_class) );

        {
            SEL op = sel_register_name( "initialize" );
            Method_ref method;

            method = Class_get_class_method( CLASS_CLASSOF(_class), op );
            if ( method )
                (*method->method_imp)( (id)_class, op );
        }
    }
}


static void __tour_install_methods_in_dtable(struct sarray* dtable, MethodList_ref method_list)
{
    int i;

    if ( !method_list )
        return ;

    if ( method_list->method_next )
        __tour_install_methods_in_dtable( dtable, method_list->method_next );

    for ( i = 0; i < method_list->method_count; ++ i ) {
        Method_ref method = &(method_list->method_list[i]);

        sarray_at_put_safe( dtable,
                            (sidx)method->method_name->sel_id,
                            method->method_imp );
    }
}


void __tour_update_dispatch_table_for_class(Class_ref _class)
{
    Class_ref next;
    struct sarray* array;

    __TOUR_RUNTIME_LOCK;

    if ( _class->dtable == __tour_uninstalled_dtable ) {
        if ( __tour_prepared_dtable_for_class( _class ) )
            __tour_prepare_dtable_for_class( _class );

        __TOUR_RUNTIME_UNLOCK;

        return ;
    }

    array = _class->dtable;
    __tour_install_premature_dtable( _class );
    sarray_free( array );

    __tour_install_dtable_for_class( _class );

    if ( _class->subclass_list ) {
        for ( next = _class->subclass_list; next; next = next->sibling_class ) {
            __tour_update_dispatch_table_for_class( next );
        }
    }

    __TOUR_RUNTIME_UNLOCK;
}


void Class_add_method_list(Class_ref _class, MethodList_ref list)
{
    assert( !list->method_next );

    __tour_register_selectors_from_list( list );

    list->method_next = _class->methods;
    _class->methods = list;

    __tour_update_dispatch_table_for_class( _class );
}


Method_ref Class_get_instance_method(Class_ref _class, SEL op)
{
    return search_for_method_in_hierarchy( _class, op );
}


Method_ref Class_get_class_method(MetaClass_ref _class, SEL op)
{
    return search_for_method_in_hierarchy( _class, op );
}


static Method_ref search_for_method_in_hierarchy(Class_ref this_class, SEL selector)
{
    Method_ref method = NULL;
    Class_ref any_class;

    if ( !sel_is_mapped( selector ) )
        return NULL;

    for ( any_class = this_class; ((!method) && any_class); any_class = CLASS_SUPER_CLASSOF(any_class) ) {
        method = search_for_method_in_list( any_class->methods, selector );
    }
    return method;
}


Method_ref search_for_method_in_list(MethodList_ref list, SEL op)
{
    MethodList_ref method_list = list;

    if ( !sel_is_mapped( op ) )
        return NULL;

    while ( method_list ) {
        int i;

        for ( i = 0; i < method_list->method_count; ++ i ) {
            Method_ref method = &method_list->method_list[i];

            if ( method->method_name ) {
                if ( method->method_name->sel_id == op->sel_id )
                    return method;
            }
                
        }
        method_list = method_list->method_next;
    }
    return NULL;
}


static retval_t __tour_forward(id object, SEL selector, arglist_t args);


static id __tour_word_forward(id receiver, SEL op, ...)
{
    void *args, *res;

    args = __builtin_apply_args();
    res = __tour_forward( receiver, op, args );

    if ( res )
        __builtin_return( res );
    else
        return res;
    
}


static double __tour_double_forward(id receiver, SEL op, ...)
{
    void *args, *res;

    args = __builtin_apply_args();
    res = __tour_forward( receiver, op, args );

    __builtin_return( res );
}


#if INVISIBLE_STRUCT_RETURN
static __big
#else
static id
#endif  /* INVISIBLE_STRUCT_RETURN */
__tour_block_forward(id receiver, SEL op, ...)
{
    void *args, *res;

    args = __builtin_apply_args();
    res = __tour_forward( receiver, op, args );

    if ( res )
        __builtin_return( res );
    else
#if INVISIBLE_STRUCT_RETURN
        return (__big) {{ 0, 0, 0, 0,  0, 0, 0, 0 }};
#else
        return nil;
#endif  /* INVISIBLE_STRUCT_RETURN */
}


static retval_t __tour_forward(id receiver, SEL selector, arglist_t args)
{
    IMP imp;
    static SEL foward_sel = NULL;
    SEL err_sel;

    if ( !foward_sel )
        foward_sel = sel_get_any_uid( "forward::" );

    if ( __tour_responds_to( receiver, foward_sel ) ) {
        imp = get_implementation( receiver, OBJ_CLASSOF(receiver), foward_sel );

        return (*imp)( receiver, foward_sel, selector, args );
    }

    err_sel = sel_get_any_uid( "doesNotRecognize:" );
    if ( __tour_responds_to( receiver, err_sel ) ) {
        imp = get_implementation( receiver, OBJ_CLASSOF(receiver), err_sel );

        return (*imp)( receiver, err_sel, selector );
    }

    {
        char msg[256 + strlen( (const char*)sel_get_name( selector ) ) + strlen( (const char*)CLASS_GET_CLASSNAME(OBJ_CLASSOF(receiver)))];

        sprintf( msg,
                 "(%s) %s does not recognize %s",
                 (CLASS_IS_META(OBJ_CLASSOF(receiver)) ? "class" : "instance"),
                 CLASS_GET_CLASSNAME(OBJ_CLASSOF(receiver)),
                 sel_get_name( selector ) );

        err_sel = sel_get_any_uid( "error:" );
        if ( __tour_responds_to( receiver, err_sel ) ) {
            imp = get_implementation( receiver, OBJ_CLASSOF(receiver), err_sel );

            return (*imp)( receiver, sel_get_any_uid( "error:" ), selector );
        }

        tour_error( receiver, TOUR_ERR_UNIMPLEMENTED, "%s\n", msg );

        return 0;
    }
}


void __tour_print_dtable_stats()
{
    int total = 0;


    __TOUR_RUNTIME_LOCK;

#ifdef TOUR_SPARSE2
    printf( "memory usage: (%s)\n", "2-level sparse arrays" );
#else
    printf( "memory usage: (%s)\n", "3-level sparse arrays" );
#endif  /* def TOUR_SPARSE2 */

    printf( "arrays: %d = %ld bytes\n",
            global_narrays,
            (long)((size_t)global_narrays * sizeof(struct sarray)) );

    total += global_narrays * sizeof(struct sarray);

    printf( "buckets: %d = %ld bytes\n",
            global_nbuckets,
            (long)((size_t)global_nbuckets * sizeof(struct sbucket)) );

    total += global_nbuckets * sizeof(struct sbucket);

    printf( "idxtables: %d = %ld bytes\n",
            global_idx_size,
            (long)((size_t)global_idx_size * sizeof(void *)) );

    total += global_idx_size * sizeof(void *);

    printf( "--------------------------------------\n" );
    printf( "total: %d bytes\n", total );
    printf( "======================================\n" );

    __TOUR_RUNTIME_UNLOCK;
}


inline struct sarray* tour_get_uninstalled_dtable()
{
    return __tour_uninstalled_dtable;
}


static cache_ptr prepared_dtable_table = NULL;


static void __tour_install_dtable_for_class(Class_ref _class)
{
    if ( !CLASS_IS_RESOLV(_class) )
        __tour_resolve_class_links();

    if ( CLASS_SUPER_CLASSOF(_class)
         && CLASS_SUPER_CLASSOF(_class)->dtable == __tour_uninstalled_dtable
         && !__tour_prepared_dtable_for_class( CLASS_SUPER_CLASSOF(_class) ) ) {
        __tour_install_dtable_for_class( CLASS_SUPER_CLASSOF(_class) );

        if ( _class->dtable != __tour_uninstalled_dtable )
            return ;
    }

    if ( __tour_prepared_dtable_for_class( _class ) )
        return ;

    __tour_prepare_dtable_for_class( _class );

    if ( CLASS_IS_CLASS(_class) )
        __tour_send_initialize( _class );
    else {
        Class_ref c = tour_lookup_class( CLASS_GET_CLASSNAME(_class) );

        assert( CLASS_IS_META(_class) );
        assert( c );

        __tour_send_initialize( c );
    }
    __tour_install_prepared_dtable_for_class( _class );
}


static void __tour_prepare_dtable_for_class(Class_ref _class)
{
    struct sarray* dtable;
    struct sarray* super_dtable;

    if ( !prepared_dtable_table ) {
        prepared_dtable_table = tour_hash_new( 32,
                                               (hash_func_type)tour_hash_ptr,
                                               (compare_func_type)tour_compare_ptrs );
    }

    if ( !CLASS_IS_RESOLV(_class) )
        __tour_resolve_class_links();

    assert( _class );
    assert( _class->dtable == __tour_uninstalled_dtable );

    dtable = __tour_prepared_dtable_for_class( _class );
    if ( 0 != dtable ) {
        tour_hash_remove( prepared_dtable_table, _class );
        sarray_free( dtable );
    }

    assert( _class != CLASS_SUPER_CLASSOF(_class) );
    if ( CLASS_SUPER_CLASSOF(_class) ) {
        if ( CLASS_SUPER_CLASSOF(_class)->dtable == __tour_uninstalled_dtable )
            __tour_install_dtable_for_class( CLASS_SUPER_CLASSOF(_class) );

        super_dtable = CLASS_SUPER_CLASSOF(_class)->dtable;

        if ( super_dtable == __tour_uninstalled_dtable )
            super_dtable = __tour_prepared_dtable_for_class( CLASS_SUPER_CLASSOF(_class) );

        assert( super_dtable );
        dtable = sarray_lazy_copy( super_dtable );
    } else
        dtable = sarray_new( __tour_selector_max_index, 0 );

    __tour_install_methods_in_dtable( dtable, _class->methods );

    tour_hash_add( &prepared_dtable_table,
                   _class,
                   dtable );
}


static struct sarray* __tour_prepared_dtable_for_class(Class_ref _class)
{
    struct sarray* dtable = NULL;

    assert( _class );

    if ( prepared_dtable_table )
        dtable = tour_hash_value_for_key( prepared_dtable_table, _class );

    return dtable;
}


static IMP __tour_get_prepared_imp(Class_ref _class, SEL selector)
{
    struct sarray* dtable;
    IMP imp;

    assert( _class );
    assert( selector );
    assert( _class->dtable == __tour_uninstalled_dtable );

    dtable = __tour_prepared_dtable_for_class( _class );

    assert( dtable );
    assert( dtable == __tour_uninstalled_dtable );

    imp = sarray_get_safe( dtable, (size_t)selector->sel_id );

    return imp;
}


static void __tour_install_prepared_dtable_for_class(Class_ref _class)
{
    assert( _class );
    assert( _class->dtable == __tour_uninstalled_dtable );

    _class->dtable = __tour_prepared_dtable_for_class( _class );

    assert( _class->dtable );
    assert( _class->dtable != __tour_uninstalled_dtable );

    tour_hash_remove( prepared_dtable_table, _class );
}
