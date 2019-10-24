#ifndef _generic_filesystem_h_
#define _generic_filesystem_h_

#if GENERIC_USE_FILESYSTEM == GENERIC_STD_FILESYSTEM
#include <filesystem>
#elif GENERIC_USE_FILESYSTEM == GENERIC_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
#else
#include <boost/filesystem.hpp>
#endif

namespace generic
{

#if GENERIC_USE_FILESYSTEM == GENERIC_STD_FILESYSTEM
namespace fs=std::filesystem;
#elif GENERIC_USE_FILESYSTEM == GENERIC_EXPERIMENTAL_FILESYSTEM
#ifdef _MSC_VER
namespace fs=std::experimental::filesystem::v1;
#else
namespace fs=std::experimental::filesystem;
#endif
#else
namespace fs=boost::filesystem;
#endif

}//namespace generic

#endif//_generic_filesystem_h_