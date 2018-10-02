#include "voxigen/simpleFilesystem.h"

#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;

namespace voxigen
{
namespace fs
{

bool exists(const char *name)
{
    bfs::path path(name);
    
    return bfs::exists(path);
}

bool is_directory(const char *name)
{
    bfs::path path(name);

    return bfs::is_directory(path);
}

bool create_directory(const char *name)
{
    bfs::path path(name);

    return bfs::create_directory(path);
}

void copy_file(const char *src, const char *dest, bool replace)
{
    bfs::path srcPath(src);
    bfs::path destPath(dest);

    bfs::copy_file(srcPath, destPath, (replace?bfs::copy_option::overwrite_if_exists:bfs::copy_option::none));
}

void VOXIGEN_EXPORT extension(const char *file, char *value, size_t &size)
{
    bfs::path path(file);

    std::string filePath=path.extension().string();

    if(value!=nullptr)
    {
        if(filePath.size()<size)
            memcpy((void *)value, filePath.data(), filePath.size()+1);
    }
}

void VOXIGEN_EXPORT get_directories(const char *directory, char **directoriePaths, size_t *sizes, size_t &size)
{
    std::vector<std::string> directories;
    bfs::path directoryPath(directory);

    for(auto &entry:bfs::directory_iterator(directoryPath))
    {
        if(bfs::is_directory(entry.path()))
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
                        memcpy((void *)directoriePaths[i], directories[i].data(), directories[i].size()+1);
                }
            }

            for(size_t i=0; i<directories.size(); ++i)
                sizes[i]=directories[i].size();
        }
    }
}

}}//namespace voxigen::fs
