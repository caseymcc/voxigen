#include "generic/fileIO.h"
#include "generic/filesystem.h"

namespace generic
{

    size_t getExtension(const char *fileName, char *extension, size_t extSize)
    {
        fs::path path(fileName);

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

}//namespace generic