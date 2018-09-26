#ifndef _generic_define_h_
#define _generic_define_h_

#ifdef generic_EXPORTS
# define generic_EXPORT __declspec(dllexport)
#else
# define generic_EXPORT
//# define generic_EXPORT __declspec(dllimport)
#endif

#endif // _generic_define_h_
