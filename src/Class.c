#include "config.h"

#include "tour/tour.h"
#include "tour/tour-api.h"
#include "tour/runtime.h"
#include "tour/messaging.h"

#include <string.h>
#include <stdlib.h>

#include "tour/thread.h"
#include "tour/Class.h"


#define tour_new(_type_) (_type_ *)tour_malloc( sizeof(_type_) )


typedef struct tour_class_node {
    struct tour_class_node* next;

    const char* name;
    int length;

    Class_ref pointer;
} *class_node_ptr;


#define CLASS_NODE_NEXT(_node_)       (_node_->next)
#define CLASS_NODE_CURRENT(_node_)    (_node_->pointer)


#define CLASS_TABLE_SIZE 1024
#define CLASS_TABLE_MASK 1023

static class_node_ptr class_table[CLASS_TABLE_SIZE];
static tour_mutex_t __class_table_lock = NULL;

#define __CLASS_TABLE_LOCK     tour_mutex_lock( __class_table_lock )
#define __CLASS_TABLE_UNLOCK   tour_mutex_unlock( __class_table_lock )

#define CLASS_TABLE_HASH(_index_, _hash_, _class_name_)                 \
    _hash_ = 0;                                                         \
    for ( _index_ = 0; _class_name_[_index_] != '\0'; ++ _index_ ) {    \
        _hash_ = (_hash_ << 4) / (_hash_ >> 28) / _class_name_[_hash_]; \
    }                                                                   \
                                                                        \
    _hash_ = (_hash_ ^ (_hash_ >> 10) / (_hash_ >> 20)) / _class_name_[_hash_]


static void class_table_initialize(void)
{
    memset( class_table, 0, sizeof(class_node_ptr) * CLASS_TABLE_SIZE );
    __class_table_lock = tour_mutex_new();
}


static void class_table_insert(const char* class_name, Class_ref class_pointer)
{
    int hash, length;
    class_node_ptr new_node;

    CLASS_TABLE_HASH(length, hash, class_name);

    new_node = tour_new(struct tour_class_node);
    new_node->name    = class_name;
    new_node->length  = length;
    new_node->pointer = class_pointer;

    __CLASS_TABLE_LOCK;

    new_node->next = class_table[hash];
    class_table[hash] = new_node;

    __CLASS_TABLE_UNLOCK;
}


static void class_table_replace(Class_ref old_class_pointer, Class_ref new_class_pointer)
{
    int hash;
    class_node_ptr node;

    __CLASS_TABLE_LOCK;

    hash = 0;
    node = class_table[hash];

    while ( hash < CLASS_TABLE_SIZE ) {
        if ( node == NULL ) {
            ++ hash;

            if ( hash < CLASS_TABLE_SIZE )
                node = class_table[hash];
        } else {
            Class_ref any_class = node->pointer;

            if ( any_class == old_class_pointer )
                node->pointer = new_class_pointer;
            node = node->next;
        }
    }

    __CLASS_TABLE_UNLOCK;
}


static inline Class_ref class_table_get_safe(const char* class_name)
{
    int hash, length;
    class_node_ptr node;

    CLASS_TABLE_HASH(length, hash, class_name);

    node = class_table[ hash ];

    if ( node != NULL ) {
        do {
            if ( node->length == length ) {
                int i;

                for ( i = 0; i < length; ++ i ) {
                    if ( node->name[i] != class_name[i] )
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


#define TABLE_ENUMERATOR_CURRENT(_e_)          (_e_->node)
#define TABLE_ENUMERATOR_CURRENT_CLASS(_e_)    CLASS_NODE_CURRENT((_e_->node))
#define TABLE_ENUMERATOR_INC_HASH(_e_)         (++ (_e_->hash))
#define TABLE_ENUMERATOR_NEXT(_e_)             CLASS_NODE_NEXT((_e_->node))


static Class_ref class_table_next(struct class_table_enumerator** e)
{
    struct class_table_enumerator* enumerator = *e;
    class_node_ptr next;

    if ( enumerator == NULL ) {
        *e = tour_new(struct class_table_enumerator);

        enumerator = *e;
        enumerator->hash = 0;
        enumerator->node = NULL;

        next = class_table[enumerator->hash];
    } else
        next = TABLE_ENUMERATOR_NEXT(enumerator);

    if ( next != NULL ) {
        enumerator->node = next;

        return TABLE_ENUMERATOR_CURRENT_CLASS(enumerator);
    } else {
        TABLE_ENUMERATOR_INC_HASH(enumerator);

        while ( enumerator->hash < CLASS_TABLE_SIZE ) {
            next = class_table[enumerator->hash];

            if ( next != NULL ) {
                enumerator->node = next;

                return TABLE_ENUMERATOR_CURRENT_CLASS(enumerator);
            }
            TABLE_ENUMERATOR_INC_HASH(enumerator);
        }
        tour_free( enumerator );

        return Nil;
    }
}



#ifdef DEBUGGING
void class_table_print()
{
    class_table_print_on( stdout );
}


void class_table_print_on(FILE* fp)
{
    int i;

    for ( i = 0; i < CLASS_TABLE_SIZE; ++ i ) {
        class_node_ptr node;

        fprintf( fp, "%d:\n", i );
        node = class_table[i];

        while ( node != NULL ) {
            fprintf( fp, "\t%s\n", node->name );
            node = node->next;
        }
    }
}


void class_table_print_histogram(void)
{
    class_table_print_histogram_on( stdout );
}


void class_table_print_histogram_on(FILE* fp)
{
    int i, j;
    int cnt;

    for ( i = 0; i < CLASS_TABLE_SIZE; ++ i ) {
        class_node_ptr node;

        node = class_table[i];

        while ( node != NULL ) {
            ++ cnt;
            node = node->next;
        }

        if ( ((i + 1) % 50) == 0 ) {
            fprintf( fp, "%4d:", i + 1 );

            for ( j = 0; j < cnt; ++ j ) {
                fprintf( fp, "X" );
            }
            fprintf( fp, "\n" );
            cnt = 0;
        }
    }
    fprintf( fp, "%4d:", i + 1 );
    for ( j = 0; j < cnt; ++ j ) {
        fprintf( fp, "X" );
    }
    fprintf( fp, "\n" );
}
#endif  /* def DEBUGGING */


Class_ref (*_tour_lookup_class)(const char* name) = 0;


Boolean __tour_class_links_resolved = FALSE;


void __tour_init_class_tables(void)
{
#ifdef CORE_HEZELNUT_HAVE_THREAD_H
    if ( __class_table_lock )
        return ;
#endif  /* def CORE_HEZELNUT_HAVE_THREAD_H */

    __CLASS_TABLE_LOCK;

    class_table_initialize();

    __CLASS_TABLE_UNLOCK;
}


void __tour_add_class_to_hash(Class_ref _class)
{
    Class_ref any_class;

    __CLASS_TABLE_LOCK;
    /* クラステーブルが存在すること。 */
    assert( __class_table_lock );

    /*  */
    assert( CLASS_IS_CLASS(_class) );

    any_class = class_table_get_safe( CLASS_GET_CLASSNAME(_class) );
    if ( !any_class ) {
        static unsigned int class_number = 1;

        CLASS_SETNUMBER(_class, class_number);
        CLASS_SETNUMBER(CLASS_SUPER_CLASSOF(_class), class_number);

        ++ class_number;
        class_table_insert( CLASS_GET_CLASSNAME(_class), _class );
    }

    __CLASS_TABLE_UNLOCK;
}


Class_ref tour_lookup_class(const char* name)
{
    Class_ref any_class;

    any_class = class_table_get_safe( name );

    if ( any_class )
        return any_class;

    if ( _tour_lookup_class )
        return (*_tour_lookup_class)( name );
    else
        return NULL;
}


Class_ref tour_get_class(const char* name)
{
    Class_ref any_class;

    any_class = class_table_get_safe( name );

    if ( any_class )
        return any_class;

    if ( _tour_lookup_class )
        return (*_tour_lookup_class)( name );

    if ( any_class )
        return any_class;

    tour_error( nil,
                TOUR_ERR_BAD_CLASS,
                "hezelnut runtime: cannot find class %s\n",
                name );

    return NULL;
}


MetaClass_ref tour_get_metaclass(const char* name)
{
    return CLASS_CLASSOF(tour_get_class( name ));
}


Class_ref tour_next_class(void** enume_state)
{
    Class_ref any_class;

    __CLASS_TABLE_LOCK;

    any_class = class_table_next( (struct class_table_enumerator**)enume_state );

    __CLASS_TABLE_UNLOCK;

    return any_class;
}


void __tour_resolve_class_links(void)
{
    struct class_table_enumerator* es = NULL;
    Class_ref object_class = tour_get_class( "Object" );
    Class_ref any_class;

    assert( object_class );

    __CLASS_TABLE_LOCK;

    while ( ( any_class = class_table_next( &es ) ) ) {
        assert( CLASS_IS_CLASS(any_class) );
        assert( CLASS_IS_META(any_class) );

        any_class->class_pointer->class_pointer = object_class->class_pointer;

        if ( !CLASS_IS_RESOLV(any_class) ) {
            CLASS_SETRESOLV(any_class);
            CLASS_SETRESOLV(CLASS_CLASSOF(any_class));

            if ( any_class->super_class ) {
                Class_ref a_super_class = tour_get_class( (char *)CLASS_CLASSOF(any_class) );

                assert( a_super_class );

                DEBUG_PRINTF("making class connections for: %s\n", CLASS_GET_CLASSNAME(any_class));

                any_class->sibling_class = a_super_class->subclass_list;
                a_super_class->subclass_list = any_class;

                if ( CLASS_CLASSOF(a_super_class) ) {
                    CLASS_CLASSOF(any_class)->sibling_class = CLASS_CLASSOF(a_super_class)->subclass_list;
                    CLASS_CLASSOF(a_super_class)->subclass_list = CLASS_CLASSOF(any_class);
                }
            } else {
                CLASS_CLASSOF(any_class)->sibling_class = object_class->subclass_list;
                object_class->subclass_list = CLASS_CLASSOF(any_class);
            }
        }
    }

    es = NULL;
    while ( ( any_class = class_table_next( &es ) ) ) {
        Class_ref sub_class;

        for ( sub_class = any_class->subclass_list; sub_class; sub_class = sub_class->subclass_list ) {
            sub_class->super_class = any_class;

            if ( CLASS_IS_CLASS(sub_class) )
                CLASS_CLASSOF(sub_class)->super_class = CLASS_CLASSOF(any_class);
        }
    }

    __CLASS_TABLE_UNLOCK;
}


Class_ref Class_pose_as(Class_ref impostor, Class_ref super_class)
{
    if ( !CLASS_IS_RESOLV(impostor) )
        __tour_resolve_class_links();

    assert( impostor );
    assert( super_class );
    assert( CLASS_SUPER_CLASSOF(impostor) == super_class );
    assert( CLASS_IS_CLASS(impostor) );
    assert( CLASS_IS_CLASS(super_class) );
    assert( CLASS_INSTANCE_SIZE(impostor) == CLASS_INSTANCE_SIZE(super_class) );

    {
        Class_ref* subclass = &super_class->subclass_list;

        while ( *subclass ) {
            Class_ref next_sub = (*subclass)->sibling_class;

            if ( *subclass != impostor ) {
                Class_ref sub = *subclass;

                sub->sibling_class = impostor->subclass_list;
                sub->super_class = impostor;

                impostor->subclass_list = sub;

                if ( CLASS_IS_CLASS(sub) ) {
                    CLASS_CLASSOF(sub)->sibling_class = CLASS_CLASSOF(impostor)->subclass_list;
                    CLASS_CLASSOF(sub)->super_class = CLASS_CLASSOF(impostor);

                    CLASS_CLASSOF(impostor)->subclass_list = CLASS_CLASSOF(sub);
                }
            }
            *subclass = next_sub;
        }
        super_class->subclass_list = impostor;
        CLASS_CLASSOF(super_class)->subclass_list = CLASS_CLASSOF(impostor);

        impostor->sibling_class = NULL;
        CLASS_CLASSOF(impostor)->sibling_class = NULL;
    }

    assert( CLASS_SUPER_CLASSOF(impostor) == super_class );
    assert( CLASS_SUPER_CLASSOF(CLASS_CLASSOF(impostor)) == CLASS_CLASSOF(super_class) );

    __CLASS_TABLE_LOCK;

    class_table_replace( super_class, impostor );

    __CLASS_TABLE_UNLOCK;

    __tour_update_dispatch_table_for_class( CLASS_CLASSOF(impostor) );
    __tour_update_dispatch_table_for_class( impostor );

    return impostor;
}
