#include "config.h"

#include <stdlib.h>

#include "tour/tour-api.h"
#include "tour/encoding.h"


#if defined(MAX)
#   undef MAX
#endif  /* defined(MAX) */

#define  MAX(_x_, _y_)                          \
    ({ typeof(_x_) __x = (_x_), __y = (_y_);    \
    ( __x > __y ? __x : __y );})

#if defined(MIN)
#   undef MIN
#endif  /* defined(MIN) */

#define  MIN(_x_, _y_)                          \
    ({ typeof(_x_) __x = (_x_), __y = (_y_);    \
    ( __x < __y ? __x : __y );})

#define ROUND(_v_, _a_)                                  \
    ({ typeof(_v_) __v = (_v_); typeof(_a_) __a = (_a_); \
        ( __a * ( __v + __a - 1 ) / __a ); })

#define    TREE_CODE(_type_)   *(_type_)
#define    TREE_TYPE(_tree_)    (_tree_)

#define    RECORD_TYPE           _C_STRUCT_B
#define    UNION_TYPE            _C_UNION_B
#define    QUAL_UNION_TYPE       _C_UNION_B
#define    ARRAY_TYPE            _C_ARY_B

#define    REAL_TYPE             _C_DBL

#define    VECTOR_TYPE           _C_VECTOR

#define    TYPE_FIELDS(_type_)                                          \
    ({ const char* _field = (_type_) + 1;                               \
    while ( *_field != _C_STRUCT_E && *_field != _C_STRUCT_B            \
            && *_field != _C_UNION_B && *_field ++ != '=' )             \
        /* do nothing */ ;                                              \
    _field; })

#define    DECL_MODE(_type_)     *(_type_)
#define    TYPE_MODE(_type_)     *(_type_)

#define    DFmode                _C_DBL

#define    get_inner_array_type(_type_)                                 \
    ( { const char* _field = (_type_);                                  \
    while ( *_field == _C_ARY_B ) {                                     \
        while ( isdigit( (unsigned char)* ++ _field ) ) ;               \
    }                                                                   \
    _field; } )


#ifndef BITS_PER_UNIT
#   define    BITS_PER_UNIT    8
#endif  /* ndef BITS_PER_UNIT */

#ifdef STRUCTURE_SIZE_BOUNDARY
#   undef    STRUCTURE_SIZE_BOUNDARY
#endif  /* def STRUCTURE_SIZE_BOUNDARY */
#define    STRUCTURE_SIZE_BOUNDARY    (BITS_PER_UNIT * sizeof(struct{char a;}))


#define    target_flags    not_target_flags
static int __attribute__ ((__unused__)) not_target_flags = 0;

#ifdef ALTIVEC_VECTOR_MODE
#   undef  ALTIVEC_VECTOR_MODE
#endif  /* def ALTIVEC_VECTOR_MODE */
#define    ALTIVEC_VECTOR_MODE(_mode_)    (0)


#define    rs6000_special_round_type_align(_struct_, _computed_, _specified_) \
    ( { const char* _fields = TYPE_FIELDS(_struct_);                    \
        (( _fields != 0                                                 \
           && TYPE_MODE(TREE_CODE(TREE_TYPE(_fields)) == ARRAY_TYPE     \
                        ? get_inner_array_type(_fields)                 \
                        : TREE_TYPE(_fields)) == DFmode)                \
         ? MAX(MAX(COMPUTED, SPECIFIED), 64)                            \
         : MAX(COMPUTED, SPECIFIED)); } )

#define    darwin_special_round_type_align(_s_, _c_, _s2_)  \
    rs6000_special_round_type_align(_s_, _c_, _s2_)


int tour_sizeof_type(const char* type)
{
    if ( *type == '"' ) {
        for ( type ++; *type ++ != '"'; ) /* do nothing */ ;
    }

    switch ( *type ) {
        case _C_BOOL:
#if __GNUC__ != 2
            return sizeof(_Bool);
            break;
#endif  /* __GNUC__ != 2 */
        case _C_ID:
            return sizeof(id);
            break;

        case _C_CLASS:
            return sizeof(Class_ref);
            break;

        case _C_SEL:
            return sizeof(SEL);
            break;

        case _C_CHR:
            return sizeof(char);
            break;

        case _C_UCHR:
            return sizeof(unsigned char);
            break;

        case _C_SHT:
            return sizeof(short);
            break;

        case _C_USHT:
            return sizeof(unsigned short);
            break;

        case _C_INT:
            return sizeof(int);
            break;

        case _C_UINT:
            return sizeof(unsigned int);
            break;

        case _C_LNG:
            return sizeof(long);
            break;

        case _C_ULNG:
            return sizeof(unsigned long);
            break;

        case _C_LNG_LNG:
            return sizeof(long long);
            break;

        case _C_ULNG_LNG:
            return sizeof(unsigned long long);
            break;

        case _C_FLT:
            return sizeof(float);
            break;

        case _C_DBL:
            return sizeof(double);
            break;

        case _C_VOID:
            return sizeof(void);
            break;

        case _C_PTR:
        case _C_ATOM:
        case _C_CHARPTR:
            return sizeof(char *);
            break;

        case _C_ARY_B:
        {
            int len = atoi( type + 1 );
            while ( isdigit( (unsigned char)* ++ type ) )
                ;
            return len * tour_aligned_size( type );
        }
        break;

        case _C_BFLD:
        {
            int position, size;
            int start_byte, end_byte;

            position = atoi( type + 1 );
            while ( isdigit( (unsigned char)* ++ type ) )
                ;
            size = atoi( type + 1 );

            start_byte = position / BITS_PER_UNIT;
            end_byte = (position + size) / BITS_PER_UNIT;

            return end_byte - start_byte;
        }
        break;

        case _C_UNION_B:
        case _C_STRUCT_B:
        {
            struct tour_struct_layout layout;
            unsigned int size;

            tour_layout_structure( type, &layout );
            while ( tour_layout_structure_next_member( &layout ) )
                /* do nothing */;
            tour_layout_finish_structure( &layout, &size, NULL );

            return size;
        }

#if __GNUC__ != 2
        case _C_COMPLEX:
        {
            type ++;

            switch ( *type ) {
                case _C_CHR:
                    return sizeof(_Complex char);
                    break;

                case _C_UCHR:
                    return sizeof(_Complex unsigned char);
                    break;

                case _C_SHT:
                    return sizeof(_Complex short);
                    break;

                case _C_USHT:
                    return sizeof(_Complex unsigned short);
                    break;

                case _C_INT:
                    return sizeof(_Complex int);
                    break;

                case _C_UINT:
                    return sizeof(_Complex unsigned int);
                    break;

                case _C_LNG:
                    return sizeof(_Complex long);
                    break;

                case _C_ULNG:
                    return sizeof(_Complex unsigned long);
                    break;

                case _C_LNG_LNG:
                    return sizeof(_Complex long long);
                    break;

                case _C_ULNG_LNG:
                    return sizeof(_Complex unsigned long long);
                    break;

                case _C_FLT:
                    return sizeof(_Complex float);
                    break;

                case _C_DBL:
                    return sizeof(_Complex double);
                    break;

                default:
                {
                    tour_error( nil, TOUR_ERR_BAD_CLASS, "unknown complex type %s\n", type );

                    return 0;
                }
            }
        }
#endif /* __GNUC__ != 2 */

        default:
            {
                    tour_error( nil, TOUR_ERR_BAD_CLASS, "unknown type %s\n", type );

                    return 0;
            }
    }
}


int tour_alignof_type(const char* type)
{
    if ( *type == '"' ) {
        for ( type ++; *type ++ != '"'; ) /* do nothing */ ;
    }

    switch ( *type ) {
        case _C_BOOL:
#if __GNUC__ != 2
            return __alignof__(_Bool);
            break;
#endif  /* __GNUC__ != 2 */
        case _C_ID:
            return __alignof__(id);
            break;

        case _C_CLASS:
            return __alignof__(Class_ref);
            break;

        case _C_SEL:
            return __alignof__(SEL);
            break;

        case _C_CHR:
            return __alignof__(char);
            break;

        case _C_UCHR:
            return __alignof__(unsigned char);
            break;

        case _C_SHT:
            return __alignof__(short);
            break;

        case _C_USHT:
            return __alignof__(unsigned short);
            break;

        case _C_INT:
            return __alignof__(int);
            break;

        case _C_UINT:
            return __alignof__(unsigned int);
            break;

        case _C_LNG:
            return __alignof__(long);
            break;

        case _C_ULNG:
            return __alignof__(unsigned long);
            break;

        case _C_LNG_LNG:
            return __alignof__(long long);
            break;

        case _C_ULNG_LNG:
            return __alignof__(unsigned long long);
            break;

        case _C_FLT:
            return __alignof__(float);
            break;

        case _C_DBL:
            return __alignof__(double);
            break;

        case _C_VOID:
            return __alignof__(void);
            break;

        case _C_PTR:
        case _C_ATOM:
        case _C_CHARPTR:
            return __alignof__(char *);
            break;

        case _C_ARY_B:
        {
            int len = atoi( type + 1 );
            while ( isdigit( (unsigned char)* ++ type ) )
                ;
            return len * tour_aligned_size( type );
        }
        break;

        case _C_BFLD:
        {
            int position, size;
            int start_byte, end_byte;

            position = atoi( type + 1 );
            while ( isdigit( (unsigned char)* ++ type ) )
                ;
            size = atoi( type + 1 );

            start_byte = position / BITS_PER_UNIT;
            end_byte = (position + size) / BITS_PER_UNIT;

            return end_byte - start_byte;
        }
        break;

        case _C_UNION_B:
        case _C_STRUCT_B:
        {
            struct tour_struct_layout layout;
            unsigned int size;

            tour_layout_structure( type, &layout );
            while ( tour_layout_structure_next_member( &layout ) )
                /* do nothing */;
            tour_layout_finish_structure( &layout, &size, NULL );

            return size;
        }

#if __GNUC__ != 2
        case _C_COMPLEX:
        {
            type ++;

            switch ( *type ) {
                case _C_CHR:
                    return __alignof__(_Complex char);
                    break;

                case _C_UCHR:
                    return __alignof__(_Complex unsigned char);
                    break;

                case _C_SHT:
                    return __alignof__(_Complex short);
                    break;

                case _C_USHT:
                    return __alignof__(_Complex unsigned short);
                    break;

                case _C_INT:
                    return __alignof__(_Complex int);
                    break;

                case _C_UINT:
                    return __alignof__(_Complex unsigned int);
                    break;

                case _C_LNG:
                    return __alignof__(_Complex long);
                    break;

                case _C_ULNG:
                    return __alignof__(_Complex unsigned long);
                    break;

                case _C_LNG_LNG:
                    return __alignof__(_Complex long long);
                    break;

                case _C_ULNG_LNG:
                    return __alignof__(_Complex unsigned long long);
                    break;

                case _C_FLT:
                    return __alignof__(_Complex float);
                    break;

                case _C_DBL:
                    return __alignof__(_Complex double);
                    break;

                default:
                {
                    tour_error( nil, TOUR_ERR_BAD_CLASS, "unknown complex type %s\n", type );

                    return 0;
                }
            }
        }
#endif /* __GNUC__ != 2 */

        default:
        {
            tour_error( nil, TOUR_ERR_BAD_CLASS, "unknown type %s\n", type );

            return 0;
        }
    }
}


int tour_aligned_size(const char* type)
{
    int size, align;

    if ( *type == '"' ) {
        for ( type ++; *type ++ != '"'; ) /* do nothing */ ;
    }

    size = tour_sizeof_type( type );
    align = tour_alignof_type( type );

    return ROUND(size, align);
}


int tour_promoted_size(const char* type)
{
    int size, word_size;

    if ( *type == '"' ) {
        for ( type ++; *type ++ != '"'; ) /* do nothing */ ;
    }

    size = tour_sizeof_type( type );
    word_size = sizeof(void *);

    return ROUND(size, word_size);
}

inline const char* tour_skip_type_qualifiers(const char* type)
{
    while ( *type == _C_CONST
            || *type == _C_IN
            || *type == _C_INOUT
            || *type == _C_OUT
            || *type == _C_BYCOPY
            || *type == _C_BYREF
            || *type == _C_ONEWAY
            || *type == _C_GCINVISIBLE ) {
        type += 1;
    }
    return type;
}


const char* tour_skip_typespec(const char* type)
{
    if ( *type == '"' ) {
        for ( type ++; *type ++ != '"'; ) /* do nothing */ ;
    }

    type = tour_skip_type_qualifiers( type );

    switch ( *type ) {
        case _C_ID:
            if ( * ++ type != '"' )
                return type;
            else {
                while ( * ++ type != '"' )
                    /* do nothing */ ;
                return type + 1;
            }

        case _C_CLASS:
        case _C_SEL:
        case _C_CHR:
        case _C_UCHR:
        case _C_SHT:
        case _C_USHT:
        case _C_INT:
        case _C_UINT:
        case _C_LNG:
        case _C_ULNG:
        case _C_LNG_LNG:
        case _C_ULNG_LNG:
        case _C_FLT:
        case _C_DBL:
        case _C_VOID:
        case _C_UNDEF:
            return ++ type;
            break;

        case _C_COMPLEX:
            return type + 2;
            break;

        case _C_ARY_B:
            while ( isdigit( (unsigned char)* ++ type ) )
                ;
            type = tour_skip_typespec( type );
            if ( *type == _C_ARY_E )
                return ++ type;
            else {
                tour_error( nil, TOUR_ERR_BAD_CLASS, "bad array type %s\n", type );

                return 0;
            }

        case _C_BFLD:
            while ( isdigit( (unsigned char)* ++ type ) )
                ;
            while ( isdigit( (unsigned char)* ++ type ) )
                ;
            return type;

        case _C_STRUCT_B:
            while ( *type != _C_STRUCT_E && *type ++ != '=' )
                ;
            while ( *type != _C_STRUCT_E ) {
                type = tour_skip_typespec( type );
            }
            return ++ type;

        case _C_UNION_B:
            while ( *type != _C_UNION_E && *type ++ != '=' )
                ;
            while ( *type != _C_UNION_E ) {
                type = tour_skip_typespec( type );
            }
            return ++ type;

        case _C_PTR:
            return tour_skip_typespec( ++ type );

        default:
        {
            tour_error( nil, TOUR_ERR_BAD_CLASS, "unknown type %s\n", type );

            return 0;
        }
    }
}


inline const char* tour_skip_offset(const char* type)
{
    if ( *type == '+' )
        type ++;
    while ( isdigit( (unsigned char)* ++ type ) )
        ;
    return type;
}


const char* tour_skip_argspec(const char* type)
{
    type = tour_skip_typespec( type );
    type = tour_skip_offset( type );

    return type;
}


int method_get_number_of_arguments(struct tour_method* mth)
{
    int i = 0;
    const char* type = mth->method_types;

    while ( *type ) {
        type = tour_skip_argspec( type );

        ++ i;
    }
    return i - 1;
}


int method_get_sizeof_arguments(struct tour_method* mth)
{
    const char* type = tour_skip_typespec( mth->method_types );

    return atoi( type );
}


char* method_get_next_argument(arglist_t arg_frame, const char** type)
{
    const char* t = tour_skip_argspec( *type );

    if ( *t == '\0' )
        return NULL;

    *type = t;
    t = tour_skip_typespec( t );

    if ( *t == '+' )
        return arg_frame->arg_regs + atoi( ++ t );
    else
        return arg_frame->arg_ptr + atoi( t );
}


char* method_get_first_argument(struct tour_method* m, arglist_t arg_frame, const char** type)
{
    *type = m->method_types;

    return method_get_next_argument( arg_frame, type );
}


char* method_get_nth_argument(struct tour_method* m, arglist_t arg_frame, int arg, const char** type)
{
    const char* t = tour_skip_argspec( m->method_types );

    if ( arg > method_get_number_of_arguments( m ) )
        return 0;

    while ( arg -- )
        t = tour_skip_argspec( t );

    *type = t;
    t = tour_skip_typespec( t );

    if ( *t == '+' )
        return arg_frame->arg_regs + atoi( ++ t );
    else
        return arg_frame->arg_ptr + atoi( t );
}


unsigned tour_get_type_qualifiers(const char* type)
{
    unsigned res = 0;
    Boolean flag = TRUE;

    while ( flag ) {
        switch ( *type ++ ) {
            case _C_CONST:        res |= _F_CONST; break;
            case _C_IN:           res |= _F_IN; break;
            case _C_INOUT:        res |= _F_INOUT; break;
            case _C_OUT:          res |= _F_OUT; break;
            case _C_BYCOPY:       res |= _F_BYCOPY; break;
            case _C_BYREF:        res |= _F_BYREF; break;
            case _C_ONEWAY:       res |= _F_ONEWAY; break;
            case _C_GCINVISIBLE:  res |= _F_GCINVISIBLE; break;

            default: flag = FALSE;
        }
    }
    return res;
}


void tour_layout_structure(const char* type, struct tour_struct_layout* layout)
{
    const char* ntype;

    if ( *type != _C_UNION_B && *type != _C_STRUCT_B ) {
        tour_error( nil,
                    TOUR_ERR_BAD_CLASS,
                    "record (or union) type expected in tour_layout_structure, got %n\n",
                    type );
    }

    type ++;
    layout->original_type = type;

    ntype = type;
    while ( *ntype != _C_STRUCT_E && *ntype != _C_STRUCT_B && *type != _C_UNION_B
            && *ntype ++ != '=' )
        /* do nothing */ ;

    if ( *(ntype - 1) == '=' )
        type = ntype;

    layout->type = type;
    layout->prev_type = NULL;
    layout->record_size = 0;
    layout->record_align = BITS_PER_UNIT;

    layout->record_align = MAX(layout->record_align, STRUCTURE_SIZE_BOUNDARY);
}


Boolean tour_layout_structure_next_member(struct tour_struct_layout* layout)
{
    register int desired_align = 0;

    register const char* bfld_type = 0;
    register int bfld_type_size,
        bfld_type_align = 0,
        bfld_field_size = 0;

    const char* type;
    Boolean unionp = layout->original_type[-1] == _C_UNION_B;

    if ( layout->prev_type ) {
        type = tour_skip_type_qualifiers( layout->prev_type );
        if ( unionp )
            layout->record_size = MAX(layout->record_size, tour_sizeof_type( type ) * BITS_PER_UNIT);
        else if ( *type != _C_BFLD )
            layout->record_size += tour_sizeof_type( type ) * BITS_PER_UNIT;
        else {
            for ( bfld_type = type + 1; isdigit( (unsigned char)*bfld_type ); ++ bfld_type )
                /* do nothing */ ;

            bfld_type_size = tour_sizeof_type( bfld_type ) * BITS_PER_UNIT;
            bfld_type_align = tour_alignof_type( bfld_type ) * BITS_PER_UNIT;
            bfld_field_size = atoi( tour_skip_typespec( bfld_type ) );

            layout->record_size += bfld_field_size;
        }
    }

    if ( *layout->type == '"' ) {
        for ( layout->type ++; *layout->type ++ != '"'; )
            /* do nothing */ ;
    }

    type = tour_skip_type_qualifiers( layout->type );

    if ( *type != _C_BFLD )
        desired_align = tour_alignof_type( bfld_type ) * BITS_PER_UNIT;
    else {
        desired_align = 1;

        for ( bfld_type = type + 1; isdigit( (unsigned char)*bfld_type ); bfld_type ++ )
            /* do nothing */ ;

        bfld_type_size = tour_sizeof_type( bfld_type ) * BITS_PER_UNIT;
        bfld_type_align = tour_alignof_type( bfld_type ) * BITS_PER_UNIT;
        bfld_field_size = atoi( tour_skip_typespec( bfld_type ) );
    }
#ifdef BIGGEST_FIELD_ALIGNMENT
    desired_align = MIN(desired_align, BIGGEST_FIELD_ALIGNMENT);
#endif  /* def BIGGEST_FIELD_ALIGNMENT */

#ifdef ADJUST_FIELD_ALIGNMENT
    desired_align = MIN(desired_align, ADJUST_FIELD_ALIGNMENT);
#endif  /* def ADJUST_FIELD_ALIGNMENT */

#ifndef PCC_BITFIELD_TYPE_MATTERS
    layout->record_align = MAX(layout->record_align, desired_align);
#else
    if ( *type == _C_BFLD ) {
        if ( bfld_field_size )
            layout->record_align = MAX(layout->record_align, desired_align);
        else
            desired_align = tour_alignof_type( bfld_type ) * BITS_PER_UNIT;
    

        {
            int type_align = bfld_type_align;
#   if 0
            if ( maximum_field_alignment != 0 )
                type_align = MIN(type_align, maximum_field_alignment);
            else if ( DECL_PACKED( field ) )
                type_align = MIN(type_align, BITS_PER_UNIT);
#   endif  /* 0 */
            layout->record_align = MAX(layout->record_align, type_align);
        }
    } else
        layout->record_align = MAX(layout->record_align, desired_align);
#endif  /* ndef PCC_BITFIELD_TYPE_MATTERS */

    if ( *type == _C_BFLD )
        layout->record_size = atoi( type + 1 );
    else if ( layout->record_size % desired_align != 0 ) {
        layout->record_size = ROUND(layout->record_size, desired_align);
    }

    layout->prev_type = layout->type;
    layout->type = tour_skip_typespec( layout->type );

    return TRUE;
}


void tour_layout_finish_structure(struct tour_struct_layout* layout, unsigned int* size, unsigned int* align)
{
    Boolean unionp = layout->original_type[-1] == _C_UNION_B;

    if ( layout->type
         && (( !unionp && *layout->type == _C_STRUCT_E )
             || ( unionp && *layout->type == _C_UNION_E ) ) ) {
#if defined(ROUND_TYPE_ALIGN) && !defined(__sparc__)
        layout->record_align = ROUND_TYPE_ALIGN(layout->original_type - 1,
                                                1,
                                                layout->record_align);
#else  /* !( defined(ROUND_TYPE_ALIGN) && !defined(__sparc__) ) */
        layout->record_align = MAX(1, layout->record_align);
#endif  /* defined(ROUND_TYPE_ALIGN) && !defined(__sparc__) */

#ifdef ROUND_TYPE_SIZE
        layout->record_align = ROUND_TYPE_ALIGN(layout->original_type,
                                                layout->record_size,
                                                layout->record_align);
#else  /* ndef ROUND_TYPE_SIZE */
        layout->record_size = ROUND(layout->record_size, layout->record_align);
#endif  /* def ROUND_TYPE_SIZE */

        layout->type = NULL;
    }
    if ( size )
        *size = layout->record_size / BITS_PER_UNIT;
    if ( align )
        *align = layout->record_align / BITS_PER_UNIT;
}


void tour_layout_structure_get_info(struct tour_struct_layout* layout, unsigned int* offset, unsigned int* align, const char** type)
{
    if ( offset )
        *offset = layout->record_size / BITS_PER_UNIT;
    if ( align )
        *align = layout->record_align / BITS_PER_UNIT;
    if ( type )
        *type = layout->prev_type;
}


// Local Variables:
//   coding: utf-8
// End:
