#ifndef tourmaline_encoding_h
#define tourmaline_encoding_h

#include "tour-api.h"
#include "Class.h"
#include <ctype.h>


_EXTERN_C_BEGIN


#define _C_CONST         'r'
#define _C_IN            'n'
#define _C_INOUT         'N'
#define _C_OUT           'o'
#define _C_BYCOPY        'O'
#define _C_BYREF         'R'
#define _C_ONEWAY        'V'
#define _C_GCINVISIBLE   '!'

#define _F_CONST         0x01
#define _F_IN            0x01
#define _F_INOUT         0x02
#define _F_OUT           0x03
#define _F_BYCOPY        0x04
#define _F_BYREF         0x08
#define _F_ONEWAY        0x10
#define _F_GCINVISIBLE   0x20


int tour_aligned_size(const char* type);


int tour_sizeof_type(const char* type);


int tour_alignof_type(const char* type);


int tour_aligned_type(const char* type);


int tour_promoted_size(const char* type);


const char* tour_skip_type_qualifiers(const char* type);


const char* tour_skip_typespec(const char* type);


const char* tour_skip_offset(const char* type);


const char* tour_skip_argspec(const char* type);


int method_get_number_of_arguments(struct tour_method* mth);


int method_get_sizeof_arguments(struct tour_method* mth);


char* method_get_first_argument( struct tour_method* mth,
                                 arglist_t arg_frame,
                                 const char** type );


char* method_get_next_argument( arglist_t arg_frame,
                                const char** type );


char* method_get_nth_argument( struct tour_method* m,
                               arglist_t arg_frame,
                               int arg,
                               const char** type );


unsigned tour_get_type_qualifiers(const char* type);


struct tour_struct_layout {
    const char*  original_type;
    const char*  type;
    const char*  prev_type;
    unsigned int record_size;
    unsigned int record_align;
};

void tour_layout_structure(const char* type, struct tour_struct_layout* layout);


Boolean tour_layout_structure_next_member(struct tour_struct_layout* layout);


void tour_layout_finish_structure(struct tour_struct_layout* layout, unsigned int* size, unsigned int* align);


void tour_layout_structure_get_info(struct tour_struct_layout* layout, unsigned int* size, unsigned int* align, const char** type);


_EXTERN_C_END


#endif  /* tourmaline_encoding_h */
