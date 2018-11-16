
#ifndef IMGLIB_EXPORT_H
#define IMGLIB_EXPORT_H

#ifdef IMGLIB_STATIC_DEFINE
#  define IMGLIB_EXPORT
#  define IMGLIB_NO_EXPORT
#else
#  ifndef IMGLIB_EXPORT
#    ifdef imglib_EXPORTS
        /* We are building this library */
#      define IMGLIB_EXPORT 
#    else
        /* We are using this library */
#      define IMGLIB_EXPORT 
#    endif
#  endif

#  ifndef IMGLIB_NO_EXPORT
#    define IMGLIB_NO_EXPORT 
#  endif
#endif

#ifndef IMGLIB_DEPRECATED
#  define IMGLIB_DEPRECATED __declspec(deprecated)
#endif

#ifndef IMGLIB_DEPRECATED_EXPORT
#  define IMGLIB_DEPRECATED_EXPORT IMGLIB_EXPORT IMGLIB_DEPRECATED
#endif

#ifndef IMGLIB_DEPRECATED_NO_EXPORT
#  define IMGLIB_DEPRECATED_NO_EXPORT IMGLIB_NO_EXPORT IMGLIB_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef IMGLIB_NO_DEPRECATED
#    define IMGLIB_NO_DEPRECATED
#  endif
#endif

#endif /* IMGLIB_EXPORT_H */
