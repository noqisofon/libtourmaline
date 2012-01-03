#include "config.h"

#include "runtime.h"
#include "Object.h"


tour_mutex_t __tour_runtime_mutex = NULL;


int __tour_runtime_threads_alive = 1;


Boolean tour_init(int* argc, char*** argv)
{
    Class_ref builtin_class;

    __tour_init_class_tables();

    
    __tour_add_class_to_hash( builtin_class );

    return TRUE;
}


void tour_quit()
{
}
