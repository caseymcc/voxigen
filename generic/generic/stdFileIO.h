#ifndef _generic_stdFileIO_h_
#define _generic_stdFileIO_h_

#include "generic/fileIO.h"

#pragma warning(push)
#pragma warning(disable:4996)

namespace generic
{
    struct StdFileIO
    {
        typedef FILE Type;
    };

    template<>
    inline typename StdFileIO::Type *open<StdFileIO>(std::string filename, std::string mode, void *userData)
    {
        return fopen(filename.c_str(), mode.c_str());
    }

    template<>
    inline void close<StdFileIO>(typename StdFileIO::Type *type)
    {
        fclose(type);
    }

    template<>
    inline size_t read<StdFileIO>(void *ptr, size_t size, size_t count, typename StdFileIO::Type *type)
    {
        return fread(ptr, size, count, type);
    }

    template<>
    inline size_t write<StdFileIO>(void *ptr, size_t size, size_t count, typename StdFileIO::Type *type)
    {
        return fwrite(ptr, size, count, type);
    }

    template<>
    inline size_t size<StdFileIO>(typename StdFileIO::Type *type)
    {
        long position;

        position=ftell(type);
        fseek(type, 0, SEEK_END);
        
        long fileSize=ftell(type);

        fseek(type, position, 0);
        return fileSize;
    }
}//namespace generic

#pragma warning(pop)

#endif //_generic_stdFileIO_h_ 