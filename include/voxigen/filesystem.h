#ifndef _voxigen_filesystem_h_
#define _voxigen_filesystem_h_

#if VOXIGEN_USE_FILESYSTEM == 1
#include <filesystem>
namespace fs=std::filesystem;
#elif VOXIGEN_USE_FILESYSTEM == 2
#include <experimental/filesystem>
#ifdef _MSC_VER
namespace fs=std::experimental::filesystem::v1;
#else
namespace fs=std::experimental::filesystem;
#endif
#else
#include <boost/filesystem.hpp>
namespace fs=boost::filesystem;
#endif

#endif //_voxigen_filesystem_h_
