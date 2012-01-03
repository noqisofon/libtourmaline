#ifndef tourmaline_ivar_h
#define tourmaline_ivar_h


_EXTERN_C_BEGIN


typedef struct tour_ivar {
    const char*    ivar_name;
    const char*    ivar_type;
    int            ivar_offset;
} *Ivar_ref;


typedef struct tour_ivar_list {

    int                 ivar_count;


    struct tour_ivar    ivar_list[1];

} IvarList, *IvarList_ref;


extern void class_ivar_set_gcinvisible(Class_ref _class, const char* ivar_name, Boolean gc_invisible);


_EXTERN_C_END


#endif  /* tourmaline_ivar_h */
