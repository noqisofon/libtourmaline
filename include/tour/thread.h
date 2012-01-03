#ifndef coreHezelnut_thread_h
#define coreHezelnut_thread_h

#include "tour.h"


_EXTERN_C_BEGIN


extern int __tour_thread_exit_status;

#define  TOUR_THREAD_INTERACTIVE_PRIORITY           2
#define  TOUR_THREAD_BACKGROUND_PRIOTITY            1
#define  TOUR_THREAD_LOW_PRIORITY                   0


typedef void* tour_thread_t;


tour_thread_t tour_thread_detach(SEL selector, id object, id argument);


void tour_thread_yield(void);


int tour_thread_exit(void);


int tour_thread_set_priority(int priority);


int tour_thread_get_priority(void);


void* tour_thread_get_data(void);


int tour_thread_set_data(void* data);


tour_thread_t tour_thread_id(void);


void tour_thread_add(void);


void tour_thread_remove(void);


struct tour_mutex {
    volatile tour_thread_t owner;
    volatile int           depth;
    void*                  backend;
};
typedef struct tour_mutex* tour_mutex_t;


tour_mutex_t tour_mutex_new(void);


int tour_mutex_free(tour_mutex_t mutex);


int tour_mutex_lock(tour_mutex_t mutex);


int tour_mutex_unlock(tour_mutex_t mutex);


int tour_mutex_trylock(tour_mutex_t mutex);


struct tour_condition {
    void*                  backend;
};
typedef struct tour_condition* tour_condition_t;


tour_condition_t tour_condition_new(void);


int tour_condition_free(tour_condition_t condition);


int tour_condition_wait(tour_condition_t condition, tour_mutex_t mutex);


int tour_condition_broadcast(tour_condition_t condition);


int tour_condition_signal(tour_condition_t condition);


typedef void(*tour_thread_callback)(void);


tour_thread_callback tour_set_thread_callback(tour_thread_callback callback);


int __tour_init_thread_system(void);


int __tour_quit_thread_system(void);


int __tour_mutex_allocate(tour_mutex_t mutex);


int __tour_mutex_deallocate(tour_mutex_t mutex);


int __tour_mutex_lock(tour_mutex_t mutex);


int __tour_mutex_trylock(tour_mutex_t mutex);


int __tour_mutex_unlock(tour_mutex_t mutex);


int __tour_condition_allocate(tour_condition_t condition);


int __tour_condition_deallocate(tour_condition_t condition);


int __tour_condition_wait(tour_condition_t condition, tour_mutex_t mutex);


int __tour_condition_broadcast(tour_condition_t condition);


int __tour_condition_signal(tour_condition_t condition);


tour_thread_t __tour_thread_detach(void (*func)(void* arg), void* arg);


void __tour_thread_yield(void);


int __tour_thread_exit(void);


int __tour_thread_set_priority(int priority);


int __tour_thread_get_priority(void);


void* __tour_thread_get_data(void);


int __tour_thread_set_data(void* data);


tour_thread_t __tour_thread_id(void);


_EXTERN_C_END


#endif  /* coreHezelnut_thread_h */
