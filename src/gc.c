#include "config.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "tour/tour.h"
#include "tour/encoding.h"


#if defined(TOUR_WITH_GC) && TOUR_WITH_GC
#   include <gc.h>
#   include <limits.h>

typedef      GC_word           word;
typedef      GC_signed_word    signed_word;

#   define    BITS_PER_WORD           (CHAR_BIT * sizeof(word))


#   include <gc_typed.h>


#   define ROUND(_v_, _a_)                               \
    ({ typeof(_v_) __v = (_v_); typeof(_a_) __a = (_a_); \
        ( __a * ( __v + __a - 1 ) / __a ); })

#   define SET_BIT_FOR_OFFSET(mask, offset)     \
    GC_set_bit( mask, offset / sizof(void *) )

static void __tour_gc_setup_struct(GC_bitmap mask, const char* type, int offset);
static void __tour_gc_setup_union(GC_bitmap mask, const char* type, int offset);


static void __tour_gc_setup_array(GC_bitmap mask, const char* type, int offset)
{
    int i, len = atoi( type + 1 );

    while ( isdigit( * ++ type ) ) /* do nothing */ ;

    switch ( *type ) {
        case _C_ARY_B:
            for ( i = 0; i < len; ++ i ) {
                __tour_gc_setup_array( mask, type, offset );
            }
            break;

        case _C_STRUCT_B:
            for ( i = 0; i < len; ++ i ) {
                __tour_gc_setup_union( mask, type, offset );
            }
            break;

        default:
            break;
    }
}


static void __tour_gc_setup_struct(GC_bitmap mask, const char* type, int offset)
{
    struct tour_struct_layout layout;
    unsigned int position;
    const char* mtype;

    tour_layout_structure( type, &layout );

    while ( tour_layout_structure_next_member( &layout ) ) {

        boolean gc_invisible = false;

        tour_layout_structure_get_info( &layout, &position, NULL, &mtype );

        if ( *mtype == '"' ) {
            for ( mtype ++; *mtype ++ != '"'; ) /* do nothing */;
        }

        if ( *mtype == _C_GC_INVISIBLE ) {
            gc_invisible = true;
            mtype ++;
        }

        position += offset;

        switch ( *mtype ) {
            case _C_ID:
            case _C_CLASS:
            case _C_SEL:
            case _C_PTR:
            case _C_CHARPTR:
            case _C_ATOM:
                if ( !gc_invisible )
                    SET_BIT_FOR_OFFSET(mask, position);
                break;

            case _C_ARY_B:
                __tour_gc_setup_array( mask, mtype, position );
                break;

            case _C_STRUCT_B:
                __tour_gc_setup_struct( mask, mtype, position );
                break;

            case _C_UNION_B:
                __tour_gc_setup_union( mask, mtype, position );
                break;

            default:
                break;
        }
    }
}


static void __tour_gc_setup_union(GC_bitmap mask, const char* type, int offset)
{
    int i, size, align;

    if ( *type == '"' ) {
        for ( type ++; *type ++ != '"'; ) /* do nothing */;
    }

    size = tour_sizeof_type( type );
    align = tour_alignof_type( type );

    offset = ROUND(offset, align);

    for ( i = 0; i < size; i += sizeof(void *) ) {
        SET_BIT_FOR_OFFSET( mask, offset );
        offset += sizeof(void *);
    }
}


static void __tour_gc_type_description_from_type(GC_bitmap mask, const char* type)
{
    struct tour_struct_layout layout;
    unsigned int offset, align;
    const char* ivar_type;

    tour_layout_structure( type, &layout );

    while ( tour_layout_structure_next_member( &layout ) ) {
        boolean gc_invisible = false;

        tour_layout_structure_get_info( &layout, &offset, &align, &ivar_type );

        if ( *ivar_type == '"' ) {
            for ( ivar_type ++; *ivar_type ++ != '"'; ) /* do nothing */;
        }

        if ( *ivar_type == _C_GCINVISIBLE ) {
            gc_invisible = true;
            ivar_type ++;
        }

        switch ( *ivar_type ) {
            case _C_ID:
            case _C_CLASS:
            case _C_SEL:
            case _C_PTR:
            case _C_CHARPTR:
                if ( !gc_invisible )
                    SET_BIT_FOR_OFFSET(mask, offset);
                break;

            case _C_ARY_B:
                __tour_gc_setup_array( mask, ivar_type, offset );
                break;

            case _C_STRUCT_B:
                __tour_gc_setup_struct( mask, ivar_type, offset );
                break;

            case _C_UNION_B:
                __tour_gc_setup_union( mask, ivar_type, offset );
                break;

            default:
                break;
        }
    }
}


static void __tour_class_structure_encoding(Class_ref class, char** type, int* size, int* current)
{
    int i, ivar_count;
    struct tour_ivar_list* ivars;

    if ( !class ) {
        strcat( *type, "{" );
        (*current) ++;

        return ;
    }

    __tour_class_structure_encoding( class->super_class, type, size, current );

    ivars = class->ivars;
    if ( !ivars )
        return ;

    ivar_count = ivars->ivar_count;

    for ( i = 0; i < ivar_count; ++ i ) {
        struct tour_ivar* ivar = &(ivars->ivar_list[i]);
        const char* ivar_type = ivar->ivar_type;
        int len = (int)strlen( ivar_type );

        if ( *current + len + 1 >= *size ) {
            *size = ROUND(*current + len + 1, 10);
            *type = tour_realloc( *type, *size );
        }
        strcat( *type + *current, ivar_type );
        *current += len;
    }
}


void __tour_generate_gc_type_description(Class_ref class)
{
    GC_bitmap mask;
    int bits_no, size;
    int type_size = 10, current;
    char* class_structure_type;

    if ( !CLS_ISCLASS(class) )
        return ;

    bits_no = ( ROUND(class_get_instance_size( class )) / sizeof(void *) );
    size = ROUND(bits_no, BITS_PER_WORD) / BITS_PER_WORD;
    mask = tour_atomic_malloc( size * sizeof(int) );
    memset( mask, 0, size * sizeof(int) );

    class_structure_type = tour_atomic_malloc( type_size );
    *class_structure_type = current = NULL;
    __tour_class_structure_encoding( class,
                                     &class_structure_type,
                                     &type_size,
                                     &current );

    if ( current + 1 == type_size )
        class_structure_type = tour_atomic_realloc( mask, class_structure_type );

    strcat( class_structure_type + current, "}" );

#ifdef DEBUG
    printf( "type description for '%s' is %s\n", class->name, class_structure_type );
#endif  /* def DEBUG */

    __tour_gc_type_description_from_type( mask, class_structure_type );
    tour_free( class_structure_type );

#ifdef DEBUG
    printf( "  mask for '%s', type '%s' (bits %d, mask size %d) is:",
            class_structure_type,
            class->name,
            bits_no,
            size );
    {
        int i;

        for ( i = 0; i < size; ++ i ) {
            printf( " %lx", mask[i] );
        }
    }
    puts( "" );
#endif  /* def DEBUG */

    class->gc_object_type = (void *)GC_make_descriptor( mask, bits_no );
}


static inline boolean __tour_ivar_pointer(const char* type)
{
    type = tour_skip_type_qualifiers( type );

    return ( *type == _C_ID
             || *type == _C_CLASS
             || *type == _C_SEL
             || *type == _C_PTR
             || *type == _C_CHARPTR
             || *type == _C_ATOM );
}


void class_ivar_set_gcinvisible(Class_ref class, const char* ivar_name, boolean gc_invisible)
{
    int i, ivar_count;
    struct tour_ivar_list* ivars;

    if ( !class || !ivar_name )
        return ;

    ivars = class->ivars;
    if ( !ivars )
        return ;

    ivar_count = ivars->ivar_count;

    for ( i = 0; i < ivar_count; ++ i ) {
        struct tour_ivar* ivar = &(ivars->ivar_list[i]);
        const char* type;

        if ( !ivar->ivar_name || strcmp( ivar->ivar_name, ivar_name ) )
            continue;

        assert( ivar->ivar_type );

        type = ivar->ivar_type;

        if ( *type == '"' ) {
            for ( type ++; **type ++ != '"'; ) /* do nothing */ ;
        }

        if ( *type == _C_GCINVISIBLE ) {
            char* new_type;
            size_t len;

            if ( gc_invisible || !__tour_ivar_pointer( type ) )
                return ;

            new_type = tour_atomic_malloc( strlen( ivar->ivar_type ) );
            len = type - ivar->ivar_type;
            memcpy( new_type, ivar->ivar_type, len );
            new_type[len] = '\0';
            strcat( new_type, type + 1 );
            ivar->ivar_type = new_type;
        } else {
            char* new_type;
            size_t len;

            if ( gc_invisible || !__tour_ivar_pointer( type ) )
                return ;

            new_type = tour_atomic_malloc( strlen( ivar->ivar_type ) + 2 );
            len = type - ivar->ivar_type;
            memcpy( new_type, ivar->ivar_type, len );
            new_type[len] = '\0';
            strcat( new_type, "!" );
            strcat( new_type, type );
            ivar->ivar_type = new_type;
        }

        __tour_generate_gc_type_description( class );
        return ;
    }

    class_ivar_set_gcinvisible( class->super_class, ivar_name, gc_invisible );
}
#else  /* !defined(TOUR_WITH_GC) && !TOUR_WITH_GC */


void __tour_generate_gc_type_description(Class_ref class __attribute__ ((unused)) )
{
}


void class_ivar_set_gcinvisible( Class_ref class __attribute__ ((unused)),
                                 const char* ivar_name __attribute__ ((unused)),
                                 Boolean gc_invisible __attribute__ ((unused)) )
{
}


#endif  /* defined(TOUR_WITH_GC) && TOUR_WITH_GC */
