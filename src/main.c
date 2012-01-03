#include "config.h"

#include <stdio.h>

#include "tour.h"
#include "Object.h"
#include "Class.h"


int main(int argc, char* argv[])
{
    tour_init( &argc, &argv );

    id foo = Object_new();

    printf( "foo is a %s\n", CLASS_GET_CLASSNAME(CLASS_CLASSOF(foo)) );

    tour_quit();

    return 0;
}
