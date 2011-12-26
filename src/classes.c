#include "tour/tour.h"
#include "tour/tour-api.h"
#include "tour/runtime.h"
#include "tour/sarray.h"
#include "tour/thread.h"


typedef struct class_node {

    struct class_node* next;

    const char* name;
    int length;

    Class pointer;

} * class_node_ptr;


#define CLASS_TABLE_SIZE  1024
#define CLASS_TABLE_MASK  1023


static class_node_ptr class_table_array[CLASS_TABLE_SIZE];


static tour_mutex_t   __class_table_lock = NULL;


#define CLASS_TABLE_HASH(_index_, _hash_, _class_name_)                 \
    _hash_ = 0;                                                         \
    for ( _index_ = 0; _class_name_ != '\0'; ++ _index_ ) {             \
        _hash_ = ( _hash_ << 4 ) ^ ( _hash_ >> 28 ) ^ _class_name_[_index_]; \
    }                                                                   \
    _hash_ = ( _hash_ ^ ( _hash_ >> 10 ) ^ ( _hash_ >> 20 ) ) & CLASS_TABLE_MASK  


static void class_table_setup(void)
{
    memset( class_table_array,
            0,
            sizeof(class_node_ptr) * CLASS_TABLE_SIZE );

    __class_table_lock = tour_mutex_allocate();
}


static void class_table_insert(const char* class_name, Class class_pointer)
{
    int hash,
        length;
    class_node_ptr new_node;

    CLASS_TABLE_HASH(length, hash, class_name);

    new_node = tour_malloc( sizeof(struct class_node) );
    new_node->name = class_name;
    new_node->length = length;
    new_node->pointer = class_pointer;

    tour_mutex_lock( __class_table_lock );

    new_node->next = class_table_array[hash];
    class_table_array[hash] = new_node;

    tour_mutex_unlock( __class_table_lock );
}


static void class_table_replace(Class old_class_pointer, Class new_class_pointer)
{
    int hash;
    class_node_ptr node;

    tour_mutex_lock( __class_table_lock );

    hash = 0;
    node = class_table_array[hash];

    while ( hash < CLASS_TABLE_SIZE ) {
        if ( node == NULL ) {
            ++ hash;
            if ( hash < CLASS_TABLE_SIZE )
                node = class_table_array[hash];
        } else {
            Class any_class = node->pointer;

            if ( any_class == old_class_pointer )
                node->pointer = new_class_pointer;

            node = node->next;
        }
    }
    
    tour_mutex_unlock( __class_table_lock );
}


static inline Class class_table_get_safe(const char* class_name)
{
    class_node_ptr node;
    int length,
        hash;

    CLASS_TABLE_HASH(length, hash, class_name);

    node = class_table_array[hash];

    if ( node != NULL ) {
        do {
            if ( node->length == length ) {
                int i;

                for ( i = 0; i < length; ++ i ) {
                    if ( (node->name)[i] != class_name[i] )
                        break;
                }

                if ( i == length )
                    return node->pointer;
            }
        } while ( ( node = node->next ) != NULL );
    }
    return Nil;
}


struct class_table_enumerator {
    int hash;
    class_node_ptr node;
};


static Class class_table_next(struct class_table_enumerator** e)
{
    struct class_table_enumerator* enumerator = *e;
    class_node_ptr next;

    if ( enumerator == NULL ) {
        *e = tour_malloc( sizeof(struct class_table_enumerator) );

        enumerator = *e;
        enumerator->hash = 0;
        enumerator->node = NULL;

        next = class_table_array[enumerator->hash];

    } else
        next = enumerator->node->next;

    if ( next != NULL ) {
        enumerator->node = next;

        return enumerator->node->pointer;
    } else {
        ++ (enumerator->hash);

        while ( enumerator->hash < CLASS_TABLE_SIZE ) {
            next = class_table_array[enumerator->hash];

            if ( next != NULL ) {
                enumerator->node = next;

                return enumerator->node->pointer;
            }
            ++ (enumerator->hash);
        }
        tour_free( enumerator );
    }
    return Nil;
}


#if defined(DEBUGGING) && DEBUGGING
void class_table_print(void)
{
    int i;

    for ( i = 0; i < CLASS_TABLE_SIZE; ++ i ) {
        class_node_ptr node;

        printf( "%d: \n", i );
        node = class_table_array[i];

        while ( node != NULL ) {
            printf( "\t%s\n", node->name );
            node = node->next;
        }
    }
}


void class_table_print_histgram(void)
{
    int i, j;
    int counter = 0;

    for ( i = 0; i < CLASS_TABLE_SIZE; ++ i ) {
        class_node_ptr node;

        node = class_table_array[i];

        while ( node != NULL ) {
            ++ counter;
            node = node->next;
        }

        if ( ( (i + 1) % 50 ) == 0 ) {
            printf( "%4d:", i + 1 );
            for ( j = 0; j < counter; ++ j ) {
                printf( "X" );
            }
            printf( "\n" );
            counter = 0;
        }
    }
    printf( "%4d:", i + 1 );
    for ( j = 0; j < counter; ++ j ) {
        printf( "X" );
    }
    printf( "\n" );
}
#endif  /* defined(DEBUGGING) && DEBUGGING */



Class (*_tour_lookup_class)(const char* name) = 0;


boolean __tour_class_links_resolved = false;


void __tour_init_class_tables(void)
{
    if ( __class_table_lock )
        return ;

    tour_mutex_lock( __tour_runtime_mutex );

    class_table_setup();

    tour_mutex_unlock( __tour_runtime_mutex );
}


void __tour_add_class_to_hash(Class class)
{
    Chass h_class;

    tour_mutex_lock( __tour_runtime_mutex );

    assert( __class_table_lock );

    assert( CLS_ISCLASS(class) );

    h_class = class_table_get_safe( class->name );

    if ( !h_class ) {
        static unsigned int class_number = 1;

        CLS_SETNUMBER(class, class_number);
        CLS_SETNUMBER(class->class_pointer, class_number);

        ++ class_number;

        class_table_insert( class->name, class );
    }
    
    tour_mutex_unlock( __tour_runtime_mutex );
}


Class tour_lookup_class(const char* name)
{
    Class class;

    class = class_table_get_safe( name );

    if ( class )
        return class;

    if ( _tour_lookup_class )
        return (*_tour_lookup_class)( name );
    else
        0;
}


Class tour_get_class(const char* name)
{
    Class class;

    class = class_table_get_safe( name );

    if ( class )
        return class;

    if ( _tour_lookup_class )
        class = (*_tour_lookup_class)( name );

    if ( class )
        return class;

    tour_error( nil,
                TOUR_ERR_BAD_CLASS,
                "tour runtime: cannot find class `%s'\n",
                name );

    return 0;
}


MetaClass tour_get_meta_class(const char* name)
{
    return tour_get_class( name )->class_pointer;
}


Class tour_next_class(void** enum_state)
{
    Class class;

    tour_mutex_lock( __tour_runtime_mutex );

    assert( __class_table_lock );

    class = class_table_next( (struct class_table_enumerator **)enum_state );

    tour_mutex_unlock( __tour_runtime_mutex );

    return class;
}


void __tour_resolve_class_links(void)
{
    struct class_table_enumerator* es = NULL;
    Class object_class = tour_get_class( "Object" );
    Class any_class;

    assert( object_class );

    tour_mutex_lock( __tour_runtime_mutex );

    while ( ( any_class = class_table_next( &es ) ) ) {
        assert( CLS_ISCLASS( any_class ) );
        assert( CLS_ISMETA( any_class->class_pointer ) );

        any_class->class_pointer->class_pointer = object_class->class_pointer;

        if ( !CLS_ISRESOLV(any_class) ) {
            CLS_SETRESOLV(any_class);
            CLS_SETRESOLV(any_class->class_pointer);

            if ( any_class->super_class ) {
                Class a_super_class = tour_get_class( any_class->super_class );

                assert( a_super_class );

                DEBUG_PRINTF("making class connections for: %s\n", any_class->name);

                any_class->sibling_class = a_super_class->subclass_list;
                a_super_class->subclass_list = any_class;

                if ( a_super_class->class_pointer ) {
                    any_class->class_pointer->sibling_class = a_super_class->class_pointer->subclass_list;
                    a_super_class->class_pointer->sibling_class = any_class->class_pointer;
                }
            } else {
                any_class->class_pointer->sibling_class = object_class->subclass_list;
                object_class->subclass_list = any_class->class_pointer;
            }
        }
    }

    es = NULL;
    while ( ( any_class = class_table_next( &es ) ) ) {
        Class sub_class;

        for ( sub_class = any_class->subclass_list; sub_class; sub_class = sub_class->subclass_list ) {
            sub_class->super_class = any_class;

            if ( CLS_ISCLASS( sub_class ) )
                sub_class->class_pointer->super_class = anu_class->class_pointer;
        }
    }

    tour_mutex_unlock( __tour_runtime_mutex );
}


#define CLASSOF(c)   ((c)->class_pointer)
#define SUPEROF(c)   ((c)->super_class)


Class class_pose_as(Class impostor, Class super_class)
{
    if ( !CLS_ISRESOLV( impostor ) )
        __tour_resolve_class_links();

    assert( impostor );
    assert( super_class );
    assert( impostor->super_class == super_class );
    assert( impostor );
    assert( CLS_ISCLASS(super_class) );
    assert( impostor->instance_size == super_class->instance_size );

    {
        Class* subclass = &(super_class->subclass_list);

        while ( *subclass ) {
            Class next_subclass = (*subclasss)->sibling_class;

            if ( *subclass != impostor ) {
                Class sub = *subclass;

                sub->sibling_class = impostor->subclass_list;
                sub->super_class = impostor;

                impostor->subclass_list = sub;

                if ( CLS_ISCLASS( sub ) ) {
                    CLASSOF(sub)->sibling_class = CLASSOF(impostor)->subclass_list;
                    CLASSOF(sub)->super_class = CLASSOF(impostor);

                    CLASSOF(impostor)->subclass_list = CLASSOF(sub);
                }
            }
            *subclass = next_subclass;
        }

        super_class->subclass_list = impostor;
        CLASSOF(super_class)->subclass_list = CLASSOF(impostor);

        impostor->sibling_class = NULL;
        CLASSOF(impostor)->sibling_class = NULL;
    }

    assert( impostor->super_class == super_class );
    assert( CLASSOF(impostor)->super_class == CLASSOF(super_class) );

    tour_mutex_lock( __tour_runtime_mutex );

    class_table_replace( super_class, impostor );

    tour_mutex_unlock( __tour_runtime_mutex );

    __tour_update_dispatch_table_for_class( CLASSOF(impostor) );
    __tour_update_dispatch_table_for_class( impostor );

    return impostor;
}
