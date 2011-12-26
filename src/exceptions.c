

#include "config.h"

#include <stdio.h>

#include "tour/tour-api.h"

#include "unwind.h"
#include "unwind-pe.h"


#define __tour_exception_class                                      \
    ((((((((_Unwind_Exception_Class)'G'                             \
           << 8 | (_Unwind_Exception_Class)'N')                     \
          << 8 | (_Unwind_Exception_Class)'U')                      \
         << 8 | (_Unwind_Exception_Class)'C')                       \
        << 8 | (_Unwind_Exception_Class)'O')                        \
       << 8 | (_Unwind_Exception_Class)'B')                         \
      << 8 | (_Unwind_Exception_Class)'J')                          \
     << 8 | (_Unwind_Exception_Class)'C')


struct Tour_Exception {
    struct _Unwind_Exception base;

    id value;

    _Unwind_Ptr landngPad;
    int handlerSwitchValue;
};


struct lsda_header_info {
    _Unwind_Ptr Start;
    _Unwind_Ptr LPStart;
    _Unwind_Ptr ttype_base;
    const unsigned char* TType;
    const unsigned char* action_table;
    unsigned char ttype_encoding;
    unsigned char call_site_encoding;
};


static const unsigned char* parse_lsda_header(struct _Unwind_Context* context, const unsigned char* p, struct lsda_header_info* info)
{
    _Unwind_Word tmp;
    unsigned char lpstart_encoding;

    info->Start = ( context ? _UNwind_GetRegionStart( context ) : 0 );

    lpstart_encoding = *p ++;
    if ( lpstart_encoding != DW_EH_PE_omit )
        p = read_encoded_value( context, lpstart_encoding, p, &info->LPStart );
    else
        info->LPStart = info->Start;

    info->ttype_encoding = *p ++;

    if ( info->ttype_encoding != DW_EH_PE_omit ) {
        p = read_uleb128( p, &tmp );
        info->TType = p + tmp;
    } else
        info->TType = 0;

    info->call_site_encoding = *p ++;
    p = read_uleb128( p, &tmp );
    info->action_table = p + tmp;

    return p;
}


static Class get_ttype_entry(struct lsda_header_info* info, _Unwind_Word i)
{
    _Unwind_Ptr ptr;

    i *= size_of_encoded_value( info->ttype_encoding,
                                info->ttype_base,
                                info->TType - i,
                                &ptr );

    if ( ptr )
        return tour_get_class( (const char *)ptr);
    else
        return NULL;
}


static int isKindOf(id value, Class target)
{
    Class c;

    if ( target == NULL )
        return 1;

    for ( c = value->class_pointer; c; c = class_get_super_class( c ) ) {
        if ( c == target )
            return 1;
    }
    return 0;
}


#ifdef SJLJ_EXCEPTIONS
#   define PERSONALITY_FUNCTION               __tour_personality_sj0
#   define __built_eh_return_data_regno(x)    x
#else
#   define PERSONALITY_FUNCTION               __tour_personality_v0
#endif  /* def SJLJ_EXCEPTIONS */


_Unwind_Reason_Code PERSONALITY_FUNCTION( int version,
                                          _Unwind_Action actions,
                                          _Unwind_Exception_Class exception_class,
                                          struct _Unwind_Exception* ue_header,
                                          struct _Unwind_Context* context )
{
    struct Tour_Exception* xh = (struct Tour_Exception *)ue_header;

    struct lsda_header_info info;
    const unsigned char* language_specific_data;
    const unsigned char* action_record;
    const unsigned char* p;
    _Unwind_Ptr landing_pad, ip;
    int handler_switch_value;
    int saw_cleanup = 0, saw_handler;
    void* return_object;

    if ( version != 1 )
        return _URC_FATAL_PHASE1_ERROR;

    if ( actions == (_UA_CLEANUP_ERROR | _UA_HANDLER_FRAME) 
         && exception_class == __tour_exception_class ) {
        handler_switch_value = xh->handlerSwitchValue;
        landing_pad = xh->landingPad;

        goto install_context;
    }

    language_specific_data = (const unsigned char *)_Unwind_GetLanguageSpecificData( context );

    if ( !language_specific_data )
        return _URC_CONTINUE_UNWIND;

    p = parse_lsda_header( context, language_specific_data, &info );
    info.ttype_base = base_of_encoded_value( info.ttype_encoding, context );
    ip = _Unwind_GetIP( context ) - 1;
    landing_pad = 0;
    action_record = 0;
    handler_switch_value = 0;

#ifdef SJLJ_EXCEPTIONS
    if ( (int)ip < 0 )
        return _URC_CONTINUE_UNWIND;
    else {
        _Unwind_Word cs_lp, cs_action;
        do {
            p = read_uleb128( p, &cs_lp );
            p = read_uleb128( p, &cs_action );
        } while ( -- ip );

        landing_pad = cs_lp + 1;
        if ( cs_action )
            action_record = info.action_table + cs_action - 1;

        goto found_someting;
    }
#else
    while ( p < info.action_table ) {
        _Unwind_Ptr cs_start, cs_len, cs_lp;
        _Unwind_Word cs_action;

        p = read_encoded_value( 0, info.call_site_encoding, p, &cs_start );
        p = read_encoded_value( 0, info.call_site_encoding, p, &cs_len );
        p = read_encoded_value( 0, info.call_site_encoding, p, &cs_lp );
        p = read_uleb128( p, &cs_action );

        if ( ip < info.Start + cs_start )
            p = info.action_table;
        else if ( ip < info.Start + cs_start + cs_len ) {
            if ( cs_lp )
                landing_pad = info.LPStart + cs_lp;
            if ( cs_action )
                action_record = info.action_table + cs_action - 1;

            goto found_someting;
        }
    }
#endif  /* def SJLJ_EXCEPTIONS */

    return _URC_CONTINUE_UNWIND;

  found_someting:
    saw_cleanup = 0;
    saw_handler = 0;

    if ( landing_pad == 0 ) {
        
    } else if ( action_record == 0 ) {
        saw_cleanup = 1;
    } else {
        _Unwind_Sword ar_filter, ar_disp;

        while ( 1 ) {
            p = action_record;
            p = read_sleb128( p, &ar_filter );

            read_sleb128( p, &ar_disp );

            if ( ar_filter == 0 )
                saw_cleanup = 1;
            else if ( ( actions & _UA_FORCE_UNWIND )
                      || exception_class != __tour_exception_class )
                ;
            else if ( ar_filter > 0 ) {

                Class catch_type = get_ttype_entry( &info, ar_filter );

                if ( isKindOf( xh->value, catch_type ) ) {
                    handler_switch_value = ar_filter;
                    saw_handler = 1;

                    break;
                }

            } else
                abort();

            if ( ar_disp == 0 )
                break;
            action_record = p + ar_disp;
        }
    }

    if ( !saw_handler && !saw_cleanup )
        return _URC_CONTINUE_UNWIND;

    if ( actions & _UA_SEARCH_PHASE ) {
        if ( !saw_handler )
            return _URC_CONTINUE_UNWIND;

        if ( exception_class == __tour_exception_class ) {
            xh->handlerSwitchValue = handler_switch_value;
            xh->landingPad = landing_pad;
        }
        return _URC_HANDLER_FOUND;
    }

  install_context:
    if ( saw_cleanup == 0 ) {
        return_object = xh->value;
        if ( !( actions & _UA_SEARCH_PHASE ) )
            _Unwind_DeleteException( &xh->base );
    }

    _Unwind_SetGR( context,
                   __builtin_eh_return_data_regno( 0 ),
                   __builtin_extend_pointer( saw_cleanup? xh: return_object ) );

    _Unwind_SetGR( context,
                   __builtin_eh_return_data_regno( 0 ),
                   handler_switch_value );

    _Unwind_SetIP( context, landing_pad );

    return _URC_INSTALL_CONTEXT;
}


static void __tour_exception_cleanup(_Unwind_Reason_Code code __attribute_((unused)), struct _Unwind_Exception* exc)
{
    free( exc );
}


void tour_exception_throw(id value)
{
    struct Tour_Exception* header = calloc( 1, sizeof(*header) );

    header->base.exception_class = __tour_exception_class;
    header->base.exception_cleanup = __tour_exception_cleanup;
    header->value = value;

#ifdef SJLJ_EXCEPTIONS
    _Unwind_SjLj_RaiseException( &header->base );
#else
    _Unwind_RaiseException( &header->base );
#endif  /* def SJLJ_EXCEPTIONS */

    abort();
}


// Local Variables:
//   coding: utf-8
// End:
