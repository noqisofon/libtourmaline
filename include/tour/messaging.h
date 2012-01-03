#ifndef tourmaline_messaging_h
#define tourmaline_messaging_h

#include "runtime.h"
#include "selector.h"


TOUR_EXPORT IMP    (*__tour_msg_forward)(SEL);
TOUR_EXPORT IMP    (*__tour_msg_forward2)(id, SEL);


IMP tour_msg_lookup(id receiver, SEL op);


static inline IMP method_get_imp(Method_ref method)
{
    return ( method != METHOD_NULL ) ? method->method_imp : (IMP)0;
}


IMP get_imp(Class_ref _class, SEL selector);


#endif  /* tourmaline_messaging_h */
