#include <pthread.h>

#include "runtime.h"
#include "thread.h"


static pthread_key_t _tour_thread_storage;
static pthread_attr_t _tour_thread_attribs;


int __tour_init_thread_system(void)
{
    /* スレッドストレージを初期化します。 */
    if ( pthread_key_create( &_tour_thread_storage, NULL ) == 0 ) {
        /*
         *
         *
         */
        if ( pthread_attr_init( &_tour_thread_attribs ) == 0 ) {
            if ( pthread_attr_setdetachstate( &_tour_thread_attribs,
                                              PTHREAD_CREATE_DETACHED ) == 0 )
                return 0;
        }
    }

    return -1;
}


int __tour_quit_thread_system(void)
{
    if ( pthread_key_delete( _tour_thread_storage ) == 0 ) {
        if ( pthread_attr_destroy( &_tour_thread_attribs ) == 0 )
            return 0;
    }
    return -1;
}


tour_thread_t __tour_thread_detach(void (*func)(void* arg), void* arg)
{
    tour_thread_t thread_id;
    pthread_t new_thread_handle;

    if ( !pthread_create( &new_thread_handle, &_tour_thread_attribs, (void *)func, arg ) )
        thread_id = *(tour_thread_t *)new_thread_handle;
    else
        thread_id = NULL;

    return thread_id;
}


int __tour_thread_set_priority(int priority)
{
    return -1;
}


int __tour_thread_get_priority(void)
{
    return TOUR_THREAD_INTERACTIVE_PRIORITY;
}


void __tour_thread_yield(void)
{
    pthread_yield( NULL );
}


int __tour_thread_exit(void)
{
    /* スレッドを終了します。 */
    pthread_exit( &__tour_thread_exit_status );

    /* もし、失敗したらここまで来ます。 */
    return -1;
}


tour_thread_t __tour_thread_id(void)
{
    pthread_t self = pthread_self();

    return *(tour_thread_t *)&self;
}


int __tour_thread_set_data(void* value)
{
    return pthread_setspecific( _tour_thread_storage, value );
}


void* __tour_thread_get_data(void)
{
    void* value = NULL;

    if ( ( value = pthread_getspecific( _tour_thread_storage ) ) != NULL )
        return value;

    return NULL;
}


int __tour_mutex_allocate(tour_mutex_t mutex)
{
    if ( pthread_mutex_init( (tour_mutex_t *)(&(mutex->backend)), NULL ) )
        return -1;
    else
        return 0;
}


int __tour_mutex_deallocate(tour_mutex_t mutex)
{
    if ( pthread_mutex_destroy( (tour_mutex_t *)(&(mutex->backend)) ) )
        return -1;
    else
        return 0;
}


int __tour_mutex_lock(tour_mutex_t mutex)
{
    return pthread_mutex_lock( (tour_mutex_t *)(&(mutex->backend)) );
}


int __tour_mutex_trylock(tour_mutex_t mutex)
{
    return pthread_mutex_trylock( (tour_mutex_t *)(&(mutex->backend)) );
}


int __tour_mutex_unlock(tour_mutex_t mutex)
{
    return pthread_mutex_unlock( (tour_mutex_t *)(&(mutex->backend)) );
}


int __tour_condition_allocate(tour_condition_t condition)
{
    if ( pthread_cond_init( (tour_condition_t *)(&(condition->backend)), NULL ) )
        return -1;
    else
        return 0;
}


int __tour_condition_deallocate(tour_condition_t condition)
{
    return pthread_cond_destroy( (tour_condition_t *)(&(condition->backend)) );
}


int __tour_condition_wait(tour_condition_t condition, tour_mutex_t mutex)
{
    return pthread_cond_wait( (tour_condition_t *)(&(condition->backend)),
                              (tour_mutex_t *)(&(mutex->backend)) );
}


int __tour_condition_broadcast(tour_condition_t condition)
{
    return pthread_cond_broadcast( (tour_condition_t *)(&(condition->backend)) );
}


int __tour_condition_signal(tour_condition_t condition)
{
    return pthread_cond_signal( (tour_condition_t *)(&(condition->backend)) );
}
