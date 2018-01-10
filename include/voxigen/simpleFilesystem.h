#ifndef _voxigen_simpleFileSystem_h_
#define _voxigen_simpleFileSystem_h_

#include "voxigen/voxigen_export.h"

#include <string>
#include <vector>

namespace voxigen
{
namespace fs
{

VOXIGEN_EXPORT bool exists(const char *name);
inline bool exists(const std::string &name) { return exists(name.c_str()); }

VOXIGEN_EXPORT bool is_directory(const char *name);
inline bool is_directory(const std::string &name) { return is_directory(name.c_str()); }

VOXIGEN_EXPORT bool create_directory(const char *name);
inline bool create_directory(const std::string &name) { return create_directory(name.c_str()); }

VOXIGEN_EXPORT void copy_file(const char *src, const char *dest, bool replace);
inline void copy_file(const std::string &src, const std::string &dest, bool replace) { copy_file(src.c_str(), dest.c_str(), replace); }

VOXIGEN_EXPORT void extension(const char *file, char *value, size_t &size);
inline std::string extension(const char *file)
{
    size_t size;
    std::string value;

    extension(file, nullptr, size);
    value.resize(size);
    extension(file, &value[0], size);

    return value;
}
inline std::string extension(const std::string &file) { return extension(file); }

VOXIGEN_EXPORT void get_directories(const char *directory, char **directories, size_t *sizes, size_t &size);
inline std::vector<std::string> get_directories(const char *directory)
{
    size_t size;

    get_directories(directory, nullptr, nullptr, size);

    std::vector<size_t> sizes;

    sizes.resize(size);
    get_directories(directory, nullptr, sizes.data(), size);

    std::vector<std::string> directories;
    std::vector<char *> directoriesChar;

    directories.resize(size);
    directoriesChar.resize(size);
    for(size_t i=0; i<size; ++i)
    {
        directories[i].reserve(sizes[i]);
        directoriesChar[i]=&(directories[i])[0];
    }

    get_directories(directory, directoriesChar.data(), sizes.data(), size);
    return directories;
}
inline std::vector<std::string> get_directories(const std::string &directory) { return get_directories(directory.c_str()); }

}
}//namespace voxigen::fs

#endif //_voxigen_simpleCamera_h_