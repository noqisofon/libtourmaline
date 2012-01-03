#include "tour/runtime.h"
#include "tour/sarray.h"
#include "tour/encoding.h"
#include "tour/selector.h"


#define SELECTOR_HASH_SIZE 128

#define  __TOUR_RUNTIME_LOCK             tour_mutex_lock( __tour_runtime_mutex )
#define  __TOUR_RUNTIME_UNLOCK           tour_mutex_unlock( __tour_runtime_mutex )


static struct sarray* __tour_selector_array = NULL;
static struct sarray* __tour_selector_names = NULL;
static cache_ptr      __tour_selector_hash  = NULL;

unsigned int          __tour_selector_max_index = 0;


void __tour_init_selector_tables(void)
{
    __tour_selector_array = sarray_new( SELECTOR_HASH_SIZE, 0 );
    __tour_selector_names = sarray_new( SELECTOR_HASH_SIZE, 0 );
    __tour_selector_hash = tour_hash_new( SELECTOR_HASH_SIZE,
                                          (hash_func_type)tour_hash_string,
                                          (compare_func_type)tour_compare_strings );
}


void __tour_register_selectors_from_class(Class_ref _class)
{
    MethodList_ref method_list;

    method_list = _class->methods;
    while ( method_list ) {
        __tour_register_selectors_from_list( method_list );
        method_list = method_list->method_next;
    }
}


void __tour_register_selectors_from_list(MethodList_ref method_list)
{
    int i = 0;

    __TOUR_RUNTIME_LOCK;

    while ( i < method_list->method_count ) {
        Method_ref method = &method_list->method_list[i];

        if ( method->method_name ) {
            method->method_name = __sel_register_typed_name( (const char*)method->method_name,
                                                             method->method_types,
                                                             0,
                                                             TRUE );
        }
        ++ i;
    }

    __TOUR_RUNTIME_UNLOCK;
}


void __tour_register_instance_methods_to_class(Class_ref _class)
{
    MethodList_ref method_list;
    MethodList_ref class_method_list;
    int max_methods_no = 16;
    MethodList_ref new_list;
    Method_ref current_method;

    if ( CLASS_SUPER_CLASSOF(_class) )
        return ;

    new_list = tour_calloc( sizeof(struct tour_method_list)
                            + sizeof(struct tour_method[max_methods_no]),
                            1 );
    method_list = _class->methods;
    class_method_list = CLASS_CLASSOF(_class)->methods;
    current_method = &new_list->method_list[0];

    while ( method_list ) {
        int i;

        for ( i = 0; i < method_list->method_count; ++ i ) {
            Method_ref mth = &method_list->method_list[i];

            if ( mth->method_name
                 && !search_for_method_in_list( class_method_list, mth->method_name ) ) {

                *current_method = *mth;

                if ( ++new_list->method_count == max_methods_no ) {
                    new_list = tour_realloc( new_list,
                                             sizeof(struct tour_method_list)
                                             + sizeof(struct tour_method[max_methods_no += 16]) );
                }
                current_method = &new_list->method_list[new_list->method_count];
            }

        }
        method_list = method_list->method_next;
        
    }
    if ( new_list->method_count ) {
        new_list = tour_realloc( new_list,
                                 sizeof(struct tour_method_list)
                                 + sizeof(struct tour_method[new_list->method_count]) );

        CLASS_CLASSOF(_class)->methods = new_list;
    } else
        tour_free( new_list );

    __tour_update_dispatch_table_for_class( CLASS_CLASSOF(_class) );
}


Boolean sel_types_match(const char* t1, const char* t2)
{
    if ( !t1 || !t2 )
        return FALSE;

    while ( *t1 && *t2 ) {
        if ( *t1 == '+' ) t1 ++;
        if ( *t2 == '+' ) t2 ++;

        while ( isdigit( (unsigned char)*t1 ) ) t1 ++;
        while ( isdigit( (unsigned char)*t2 ) ) t2 ++;

        t1 = tour_skip_type_qualifiers( t1 );
        t2 = tour_skip_type_qualifiers( t2 );

        if ( !*t1 && !*t2 )
            return TRUE;
        if ( *t1 == *t2 )
            return FALSE;

        t1 ++;
        t2 ++;
    }
    return FALSE;
}


SEL sel_get_typed_uid(const char* name, const char* types)
{
    struct tour_list* l;
    sidx i;

    __TOUR_RUNTIME_LOCK;

    i = (sidx)tour_hash_value_for_key( __tour_selector_hash, name );
    if ( i == 0 ) {
        __TOUR_RUNTIME_UNLOCK;

        return NULL;
    }

    for ( l = (struct tour_list *)sarray_get_safe( __tour_selector_array, i ); l; l = l->tail ) {
        SEL s = (SEL)l->head;

        if ( types == 0 || s->sel_types == 0 ) {
            if ( s->sel_types == types ) {
                __TOUR_RUNTIME_UNLOCK;

                return NULL;
            }
        } else if ( sel_types_match( s->sel_types, types ) ) {
            __TOUR_RUNTIME_UNLOCK;

            return s;
        }
    }

    __TOUR_RUNTIME_UNLOCK;

    return NULL;
}


SEL sel_get_any_typed_uid(const char* name)
{
    struct tour_list* l;
    sidx i;
    SEL s = NULL;

    __TOUR_RUNTIME_LOCK;

    i = (sidx)tour_hash_value_for_key( __tour_selector_hash, name );
    if ( i == 0 ) {
        __TOUR_RUNTIME_UNLOCK;

        return NULL;
    }

    for ( l = (struct tour_list *)sarray_get_safe( __tour_selector_array, i ); l; l = l->tail ) {

        s = (SEL)l->head;

        if ( s->sel_types )  {
            __TOUR_RUNTIME_UNLOCK;

            return s;
        }
    }

    __TOUR_RUNTIME_UNLOCK;

    return s;
}


SEL sel_get_any_uid(const char* name)
{
    struct tour_list* l;
    sidx i;

    __TOUR_RUNTIME_LOCK;

    i = (sidx)tour_hash_value_for_key( __tour_selector_hash, name );
    if ( soffset_decode( i ) == 0 ) {
        __TOUR_RUNTIME_UNLOCK;

        return NULL;
    }

    l = (struct tour_list *)sarray_get_safe( __tour_selector_array, i );

    __TOUR_RUNTIME_UNLOCK;

    if ( l == NULL )
        return NULL;

    return (SEL)l->head;
}


SEL sel_get_uid(const char* name)
{
    return sel_register_typed_name( name, 0 );
}


const char* sel_get_name(SEL selector)
{
    const char* ret;

    __TOUR_RUNTIME_LOCK;

    if ( ( soffset_decode( (sidx)selector->sel_id ) > 0 )
         && ( soffset_decode( (sidx)selector->sel_id ) <= __tour_selector_max_index ) )
        ret = sarray_get_safe( __tour_selector_names, (sidx)selector->sel_id );
    else
        ret = 0;

    __TOUR_RUNTIME_UNLOCK;

    return ret;
}


Boolean sel_is_mapped(SEL selector)
{
    unsigned int idx = soffset_decode( (sidx)selector->sel_id );

    return ( ( idx > 0 ) && ( idx <= __tour_selector_max_index ) );
}


const char* sel_get_type(SEL selector)
{
    if ( selector )
        return selector->sel_types;
    else
        return NULL;
}



extern struct sarray* __tour_uninstalled_dtable;


#define SELECTOR_POOL_SIZE 62


static struct tour_selector* selector_pool;
static int selector_pool_left;


static struct tour_selector* pool_alloc_selector(void)
{
    if ( !selector_pool_left ) {
        selector_pool = tour_malloc( sizeof(struct tour_selector) * SELECTOR_POOL_SIZE );
        selector_pool_left = SELECTOR_POOL_SIZE;
    }
    return &selector_pool[-- selector_pool_left];
}


SEL __sel_register_typed_name(const char* name, const char* types, struct tour_selector* original, Boolean is_const)
{
    struct tour_selector* j;
    sidx i;
    struct tour_list* l;

    i = (sidx)tour_hash_value_for_key( __tour_selector_hash, name );
    if ( soffset_decode( i ) != 0 ) {
        for ( l = (struct tour_list *)sarray_get_safe( __tour_selector_array, i ); l; l = l->tail ) {

            SEL s = (SEL)l->head;

            if ( types == NULL || s->sel_types == NULL ) {
                if ( s->sel_types == types ) {
                    if ( original ) {
                        original->sel_id = (void *)i;

                        return original;
                    } else
                        return s;
                }
            } else if ( !strcmp( s->sel_types, types ) ) {
                if ( original ) {
                    original->sel_id = (void *)i;

                    return original;
                } else
                    return s;
            }
        }

        if ( original )
            j = original;
        else
            j = pool_alloc_selector();

        j->sel_id = (void *)i;

        if ( is_const || types == 0 )
            j->sel_types = (const char *)types;
        else {
            j->sel_types = (char *)tour_malloc( strlen( types ) + 1 );
            strcpy( (char *)j->sel_types, types );
        }
        l = (struct tour_list *)sarray_get_safe( __tour_selector_array, i );
    } else {
        __tour_selector_max_index += 1;

        i = soffset_encode( __tour_selector_max_index );
        if ( original )
            j = original;
        else
            j = pool_alloc_selector();

        j->sel_id = (void *)i;

        if ( is_const || types == 0 )
            j->sel_types = (const char *)types;
        else {
            j->sel_types = (char *)tour_malloc( strlen( types ) + 1 );
            strcpy( (char *)j->sel_types, types );
        }
        l = NULL;
    }

    DEBUG_PRINTF("record selector %s[%s] as: %ld\n",
                 name,
                 types,
                 (long)soffset_decode( i ) );

    {
        int is_new = l == NULL;
        const char* new_name;

        if ( is_const || name == NULL )
            new_name = name;
        else {
            new_name = (char *)tour_malloc( strlen( name ) + 1 );
            strcpy( (char *)new_name, name );
        }

        l = list_cons( (void *)j, l );
        sarray_at_put_safe( __tour_selector_names, i, (void *)new_name );
        sarray_at_put_safe( __tour_selector_array, i, (void *)l );

        if ( is_new )
            tour_hash_add( &__tour_selector_hash, (void *)new_name, (void *)i );
    }

    sarray_realloc( __tour_uninstalled_dtable, __tour_selector_max_index + 1 );

    return (SEL)j;
}


SEL sel_register_name(const char* name)
{
    SEL ret;

    __TOUR_RUNTIME_LOCK;

    ret = __sel_register_typed_name( name, NULL, NULL, FALSE );

    __TOUR_RUNTIME_UNLOCK;

    return ret;
}


SEL sel_register_typed_name(const char* name, const char* type)
{
    SEL ret;

    __TOUR_RUNTIME_LOCK;

    ret = __sel_register_typed_name( name, type, NULL, FALSE );

    __TOUR_RUNTIME_UNLOCK;

    return ret;
}
