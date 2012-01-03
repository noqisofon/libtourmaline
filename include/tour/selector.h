#ifndef tourmaline_selector_h
#define tourmaline_selector_h


#define METHOD_NULL   (Method_ref)0


struct tour_method_description {
    SEL name;
    char* types;
};


typedef struct tour_method {
    SEL      method_name;

    const char* method_types;

    IMP method_imp;
} Method, *Method_ref;


typedef struct tour_method_list {
    struct tour_method_list*    method_next;

    int                         method_count;

    struct tour_method          method_list[1];

} *MethodList_ref;


const char* sel_get_name(SEL selector);


const char* sel_get_type();


SEL sel_get_uid(const char* name);


SEL sel_get_any_uid(const char* name);


SEL sel_get_any_typed_uid(const char* name);


SEL sel_get_typed_uid(const char* name, const char*);


SEL sel_register_name(const char* name);


SEL sel_register_typed_name(const char* name, const char* type);


Boolean sel_is_mapped(SEL a_sel);


#endif  /* tourmaline_selector_h */
