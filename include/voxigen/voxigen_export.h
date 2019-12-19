#ifndef VOXIGEN_EXPORT_H
#define VOXIGEN_EXPORT_H

#ifdef VOXIGEN_STATIC_DEFINE
#  define VOXIGEN_EXPORT
#  define VOXIGEN_NO_EXPORT
#else
#   define VOXIGEN_EXPORT 
#  ifndef VOXIGEN_EXPORT
#    ifdef VOXIGEN_EXPORTS
/* We are building this library */
#      define VOXIGEN_EXPORT __declspec(dllexport)
#    else
/* We are using this library */
#      define VOXIGEN_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef VOXIGEN_NO_EXPORT
#    define VOXIGEN_NO_EXPORT 
#  endif
#endif

#ifndef VOXIGEN_DEPRECATED
#  define VOXIGEN_DEPRECATED __declspec(deprecated)
#endif

#ifndef VOXIGEN_DEPRECATED_EXPORT
#  define VOXIGEN_DEPRECATED_EXPORT VOXIGEN_EXPORT VOXIGEN_DEPRECATED
#endif

#ifndef VOXIGEN_DEPRECATED_NO_EXPORT
#  define VOXIGEN_DEPRECATED_NO_EXPORT VOXIGEN_NO_EXPORT VOXIGEN_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef VOXIGEN_NO_DEPRECATED
#    define VOXIGEN_NO_DEPRECATED
#  endif
#endif

#endif /* VOXIGEN_EXPORT_H */