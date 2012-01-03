#ifndef tourmaline_tour_list_h
#define tourmaline_tour_list_h


_EXTERN_C_BEGIN


struct tour_list {
    void* head;
    struct tour_list* tail;
};


static inline struct tour_list* list_cons(void* head, struct tour_list* tail)
{
    struct tour_list* cel;

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


_EXTERN_C_END


#endif  /* tourmaline_tour_list_h */
