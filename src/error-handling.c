#include "config.h"
#include <stdlib.h>

#include "tour/runtime.h"


static tour_error_handler _tour_error_handler = NULL;


#define TOUR_ERROR_HANDLE(_object_, _code_, _format_, _argument_)   (*_tour_error_handler)( _object_, _code_, _format_, _argument_ )


void tour_error(id object, int code, const char* format, ...)
{
    va_list argument;

    va_start(argument, format);
    tour_verror( object, code, format, argument );
    va_end(argument);
}


void tour_verror(id object, int code, const char* format, va_list argument)
{
    Boolean result = FALSE;

    if ( _tour_error_handler )
        result = TOUR_ERROR_HANDLE(object, code, format, argument);
    else
        vfprintf( stderr, format, argument );

    if ( result )
        return ;
    else
        abort();
}


tour_error_handler tour_set_error_handler(tour_error_handler handler)
{
    tour_error_handler temp = _tour_error_handler;

    _tour_error_handler = handler;

    return temp;
}
