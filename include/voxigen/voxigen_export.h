#if defined(_WIN32) || defined(_WIN64)
#   ifndef VOXIGEN_EXPORT
#     ifdef voxigen_EXPORTS
#       define VOXIGEN_EXPORT __declspec(dllexport)
#     else
#       define VOXIGEN_EXPORT __declspec(dllimport)
#     endif
#   endif
#else
#   define VOXIGEN_EXPORT 
#endif

#ifndef VOXIGEN_NO_EXPORT
#  define VOXIGEN_NO_EXPORT 
#endif
