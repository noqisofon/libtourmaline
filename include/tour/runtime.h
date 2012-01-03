#ifndef coreHezelnut_runtime_h
#define coreHezelnut_runtime_h

#include <stdarg.h>

#include <stdio.h>
#include <ctype.h>

#include <stddef.h>
#include <assert.h>

#include "tour.h"
#include "tour-api.h"

#include "thread.h"

#include "hash.h"
#include "tour-list.h"


_EXTERN_C_BEGIN


typedef Class_ref (*_tour_class_initialize)(const char*);


extern void __tour_add_class_to_hash(Class_ref);

extern void __tour_init_selector_tables(void);
extern void __tour_init_class_tables(void);
extern void __tour_init_dispatch_tables(void);

extern void __tour_install_premature_dtable(Class_ref);

extern void __tour_resolve_class_links(void);

extern void __tour_register_selectors_from_class(Class_ref);
extern void __tour_register_selectors_from_list(MethodList_ref);

extern void __tour_update_dispatch_table_for_class(Class_ref);

extern void class_add_method_list(Class_ref, MethodList_ref);

extern void __objc_register_instance_methods_to_class(Class_ref);
extern Method_ref search_for_method_in_list(MethodList_ref list, SEL op);


extern unsigned int __tour_selector_max_index;

extern tour_mutex_t __tour_runtime_mutex;


extern int __tour_runtime_threads_alive;


#ifdef DEBUG_RUNTIME
#   define DEBUG_PRINTF(format, args...) printf( format, ## args )
#else
#   define DEBUG_PRINTF(format, args...) 
#endif  /* def DEBUG_RUNTIME */


Boolean __tour_responds_to(id object, SEL selector);
SEL __sel_register_typed_name( const char*,
                               const char*,
                               struct tour_selector*,
                               Boolean is_const );
extern void __tour_generate_gc_type_description(Class_ref);


_EXTERN_C_END


#endif  /* coreHezelnut_runtime_h */
