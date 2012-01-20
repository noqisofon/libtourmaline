#include "tour/tour.h"
#include "tour/Object.h"
#include "tour/Class.h"


int main()
{
    SEL s = NULL;

    __tour_init_selector_tables();

    s = sel_get_any_typed_uid( "copy" );

    if ( !s )
        printf( "`copy' not found\n" );
    else
        printf( "%s: %p\n", s->sel_types, s->sel_id );
    
    return 0;
}
