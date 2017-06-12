
#ifndef VOXIGEN_EXPORT
#  ifdef voxigen_EXPORTS
#    define VOXIGEN_EXPORT __declspec(dllexport)
#  else
#    define VOXIGEN_EXPORT __declspec(dllimport)
#  endif
#endif

#ifndef VOXIGEN_NO_EXPORT
#  define VOXIGEN_NO_EXPORT 
#endif
