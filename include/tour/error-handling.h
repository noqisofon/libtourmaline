#ifndef tourmaline_error_handling_h
#define tourmaline_error_handling_h


_EXTERN_C_BEGIN


extern void tour_error(id object, int code, const char* format, ...);
extern void tour_verror(id object, int code, const char* format, va_list argument);

typedef Boolean (*tour_error_handler)(id, int, const char*, va_list);

extern tour_error_handler tour_set_error_handler(tour_error_handler handler);


#define TOUR_ERR_UNKNOWN            1

#define TOUR_ERR_MEMORY            10

#define TOUR_ERR_RECURSE_ROOT      20

#define TOUR_ERR_BAD_CLASS         23

#define TOUR_ERR_UNIMPLEMENTED     30

#define TOUR_ERR_BAD_STATE         40


_EXTERN_C_END


#endif  /* tourmaline_error_handling_h */
