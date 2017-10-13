#include "voxigen/simpleFileSystem.h"

#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;

namespace voxigen
{
namespace fs
{

bool exists(std::string name)
{
    bfs::path path(name);
    
    return bfs::exists(path);
}

bool is_directory(std::string name)
{
    bfs::path path(name);

    return bfs::is_directory(path);
}

bool create_directory(std::string name)
{
    bfs::path path(name);

    return bfs::create_directory(path);
}

void copy_file(std::string src, std::string dest, bool replace)
{
    bfs::path srcPath(src);
    bfs::path destPath(dest);

    bfs::copy_file(srcPath, destPath, (replace?bfs::copy_option::overwrite_if_exists:bfs::copy_option::none));
}

std::string VOXIGEN_EXPORT extension(std::string file)
{
    bfs::path path(file);

    return path.extension().string();
}

std::vector<std::string> get_directories(std::string directory)
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

    return directories;
}

}}//namespace voxigen::fs
