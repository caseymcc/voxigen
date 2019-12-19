#ifndef _generic_fileIO_h_
#define _generic_fileIO_h_

#include "generic/generic_export.h"

#include <string>

namespace generic
{
    template<typename _FileIO>
    typename _FileIO::Type *open(std::string filename, std::string mode, void *userData=nullptr)
    {
        return (_FileIO::Type *)nullptr;
    }

    template<typename _FileIO>
    void close(typename _FileIO::Type *type)
    {
    }

    template<typename _FileIO>
    size_t read(void *ptr, size_t size, size_t count, typename _FileIO::Type *type)
    {
        return 0;
    }

    template<typename _FileIO>
    size_t write(void *ptr, size_t size, size_t count, typename _FileIO::Type *type)
    {
        return 0;
    }

    template<typename _FileIO>
    size_t size(typename _FileIO::Type *type)
    {
        return 0;
    }

    GENERIC_EXPORT size_t getExtension(const char *fileName, char *ext, size_t extSize);

    inline std::string getExtension(std::string fileName)
    {
        //guess 4 and less plus "."
        std::string ext(5, 0);

        size_t extRead=generic::getExtension(fileName.c_str(), (char *)ext.data(), ext.size());

        if(extRead>5)
        {
            //if it is larger than guessed, resize and read again
            ext.resize(extRead);
            extRead=generic::getExtension(fileName.c_str(), (char *)ext.data(), ext.size());
        }
        else
            ext.resize(extRead);

        return ext;
    }

}//namespace generic

#endif //_generic_fileIO_h_ 