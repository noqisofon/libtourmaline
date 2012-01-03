#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(libtourmaline, 0.1, ned.rihine@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/Class.c])
AC_CONFIG_HEADERS([include/config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB(pthread, pthread_create, [LIBS="$LIBS -pthread"])
AC_CHECK_LIB(gc, GC_malloc, [LIBS="$LIBS -lgc"])

# Checks for header files.
AC_CHECK_HEADERS([limits.h stddef.h stdlib.h string.h])
AC_CHECK_HEADERS(pthread.h,, [AC_MSG_ERROR([pthread.h required])])
AC_CHECK_HEADERS(gc.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_PROG_RANLIB
AC_CHECK_FUNCS([memset])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
