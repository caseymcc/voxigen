#ifndef _generic_define_h_
#define _generic_define_h_

#ifdef GENERIC_STATIC_DEFINE
#  define GENERIC_EXPORT
#  define GENERIC_NO_EXPORT
#else
#  ifndef GENERIC_EXPORT
#    ifdef GENERIC_EXPORTS
/* We are building this library */
#      define GENERIC_EXPORT 
#    else
/* We are using this library */
#      define GENERIC_EXPORT 
#    endif
#  endif

#  ifndef GENERIC_NO_EXPORT
#    define GENERIC_NO_EXPORT 
#  endif
#endif

#ifndef GENERIC_DEPRECATED
#  define GENERIC_DEPRECATED __declspec(deprecated)
#endif

#ifndef GENERIC_DEPRECATED_EXPORT
#  define GENERIC_DEPRECATED_EXPORT GENERIC_EXPORT GENERIC_DEPRECATED
#endif

#ifndef GENERIC_DEPRECATED_NO_EXPORT
#  define GENERIC_DEPRECATED_NO_EXPORT GENERIC_NO_EXPORT GENERIC_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GENERIC_NO_DEPRECATED
#    define GENERIC_NO_DEPRECATED
#  endif
#endif

#endif // _generic_define_h_