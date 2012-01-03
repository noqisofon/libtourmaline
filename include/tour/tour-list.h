#ifndef tourmaline_tour_list_h
#define tourmaline_tour_list_h

#include "allocation.h"


_EXTERN_C_BEGIN


struct tour_list {
    void* head;
    struct tour_list* tail;
};


#define list_car(list)  ((list)->head)
#define list_cdr(list)  ((list)->tail)


static inline struct tour_list* list_cons(void* head, struct tour_list* tail)
{
    struct tour_list* cell;

    cell = (struct tour_list *)tour_malloc( sizeof(struct tour_list) );
    cell->head = head;
    cell->tail = tail;

    return cell;
}


static inline size_t list_length(struct tour_list* list)
{
    size_t i = 0;

    while ( list ) {
        ++ i;
        list = list->tail;
    }
    return i;
}


static inline void* list_nth(int index, struct tour_list* list)
{
    while ( index -- != 0 ) {
        if ( list->tail )
            list = list->tail;
        else
            return NULL;
    }
    return list->head;
}


static inline void list_remove_head(struct tour_list** list)
{
    if ( (*list)->tail ) {
        struct tour_list* tail = (*list)->tail;

        *(*list) = *tail;
        tour_free( tail );
    } else {
        tour_free( *list );
        (*list) = NULL;
    }
}


static inline void list_remove_elem(struct tour_list** list, void* elem)
{
    while ( *list ) {
        if ( (*list)->head == elem )
            list_remove_head( list );
        list = &((*list)->tail);
    }
}


static inline void list_mapcar(struct tour_list* list, void(*function)(void*))
{
    while ( list ) {
        (*function)( list->head );
        list = list->tail;
    }
}


static inline struct tour_list** list_find(struct tour_list** list, void* elem)
{
    while ( *list ) {
        if ( (*list)->head == elem )
            return list;
        list = &((*list)->tail);
    }
    return NULL;
}


static inline void list_free(struct tour_list* list)
{
    if ( list ) {
        list_free( list->tail );
        tour_free( list );
    }
}


_EXTERN_C_END


#endif  /* tourmaline_tour_list_h */
