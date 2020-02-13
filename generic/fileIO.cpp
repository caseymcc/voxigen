#include "generic/fileIO.h"
#include "generic/filesystem.h"

#ifdef _WIN32
#include <windows.h>
#else
#endif

#include <cassert>

namespace generic
{
namespace io
{

size_t getExtension(const char *fileName, char *extension, size_t extSize)
{
    generic::fs::path path(fileName);

    if(!path.has_extension())
    {
        extension[0]=0;
        return 0;
    }

    std::string ext=path.extension().string();

    if(ext.size()<extSize)
        strcpy(extension, ext.c_str());
    return ext.size();
}

}//namespace io
}//namespace generic