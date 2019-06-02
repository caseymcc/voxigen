#include "voxigen/simpleFilesystem.h"
#include "voxigen/filesystem.h"

#include <cstring>

namespace voxigen
{
namespace fs
{

void current_path(char *value, size_t &size)
{
    ::fs::path path=::fs::current_path();
    std::string pathString=path.string();

    if((value!=nullptr) && (pathString.size()<size))
        std::memcpy((void *)value, pathString.data(), pathString.size()+1);
    size=pathString.size()+1;
}

bool exists(const char *name)
{
    ::fs::path path(name);
    
    return ::fs::exists(path);
}

bool is_directory(const char *name)
{
    ::fs::path path(name);

    return ::fs::is_directory(path);
}

bool create_directory(const char *name)
{
    ::fs::path path(name);

    return ::fs::create_directory(path);
}

void copy_file(const char *src, const char *dest, bool replace)
{
    ::fs::path srcPath(src);
    ::fs::path destPath(dest);

#if VOXIGEN_USE_FILESYSTEM == 0
    ::fs::copy_file(srcPath, destPath, (replace?::fs::copy_option::overwrite_if_exists : ::fs::copy_option::none));
#else
    ::fs::copy_file(srcPath, destPath, (replace?::fs::copy_options::overwrite_existing : ::fs::copy_options::none));
#endif
}

void VOXIGEN_EXPORT extension(const char *file, char *value, size_t &size)
{
    ::fs::path path(file);

    std::string filePath=path.extension().string();

    if((value!=nullptr) && (filePath.size()<size))
        std::memcpy((void *)value, filePath.data(), filePath.size()+1);
    size=filePath.size()+1;
}

void VOXIGEN_EXPORT get_directories(const char *directory, char **directoriePaths, size_t *sizes, size_t &size)
{
    std::vector<std::string> directories;
    ::fs::path directoryPath(directory);

    for(auto &entry : ::fs::directory_iterator(directoryPath))
    {
        if(::fs::is_directory(entry.path()))
        {
            directories.push_back(entry.path().stem().string());
        }
    }

    size=directories.size();
    if(sizes!=nullptr)
    {
        if(directories.size()<=size)
        {
            if(directoriePaths!=nullptr)
            {
                for(size_t i=0; i<directories.size(); ++i)
                {
                    if(directories[i].size()<sizes[i])
                        std::memcpy((void *)directoriePaths[i], directories[i].data(), directories[i].size()+1);
                }
            }

            for(size_t i=0; i<directories.size(); ++i)
                sizes[i]=directories[i].size();
        }
    }
}

}}//namespace voxigen::fs
