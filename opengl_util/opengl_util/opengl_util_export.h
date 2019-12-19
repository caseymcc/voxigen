
#ifndef OPENGL_UTIL_EXPORT_H
#define OPENGL_UTIL_EXPORT_H

#ifdef OPENGL_UTIL_STATIC_DEFINE
#  define OPENGL_UTIL_EXPORT
#  define OPENGL_UTIL_NO_EXPORT
#else
#  ifndef OPENGL_UTIL_EXPORT
#    ifdef opengl_util_EXPORTS
        /* We are building this library */
#      define OPENGL_UTIL_EXPORT 
#    else
        /* We are using this library */
#      define OPENGL_UTIL_EXPORT 
#    endif
#  endif

#  ifndef OPENGL_UTIL_NO_EXPORT
#    define OPENGL_UTIL_NO_EXPORT 
#  endif
#endif

#ifndef OPENGL_UTIL_DEPRECATED
#  define OPENGL_UTIL_DEPRECATED __declspec(deprecated)
#endif

#ifndef OPENGL_UTIL_DEPRECATED_EXPORT
#  define OPENGL_UTIL_DEPRECATED_EXPORT OPENGL_UTIL_EXPORT OPENGL_UTIL_DEPRECATED
#endif

#ifndef OPENGL_UTIL_DEPRECATED_NO_EXPORT
#  define OPENGL_UTIL_DEPRECATED_NO_EXPORT OPENGL_UTIL_NO_EXPORT OPENGL_UTIL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef OPENGL_UTIL_NO_DEPRECATED
#    define OPENGL_UTIL_NO_DEPRECATED
#  endif
#endif

#endif /* OPENGL_UTIL_EXPORT_H */
