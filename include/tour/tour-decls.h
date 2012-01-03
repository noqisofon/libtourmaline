#ifndef tourmaline_tour_decls_h
#define tourmaline_tour_decls_h


#if !defined(__MINGW32__) && ( defined(_WIN32) || defined(__WIN32__) || defined(WIN32) )

#  define TOUR_EXPORT         extern __declspec(dllexport)
#  define TOUR_DECLARE        extern __declspec(dllimport)

#else

#  define TOUR_EXPORT         extern
#  define TOUR_DECLARE        

#endif  /* !define(__MINGW32__) && ( defined(_WIN32) || defined(__WIN32__) || defined(WIN32) ) */


#endif  /* tourmaline_tour_decls_h */
