#ifndef _generic_stdFileIO_h_
#define _generic_stdFileIO_h_

#include "generic/fileIO.h"
#include "generic/filesystem.h"

#include <cassert>

#pragma warning(push)
#pragma warning(disable:4996)

namespace generic
{
namespace io
{

struct StdFileIO
{
    typedef FILE Type;
    typedef generic::fs::file_time_type Time;
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

template<>
inline bool exists<StdFileIO>(const std::string &fileName)
{
    return generic::fs::exists(generic::fs::path(fileName));
}

template<>
inline bool create_directory<StdFileIO>(const std::string &directoryName)
{
    return generic::fs::create_directory(generic::fs::path(directoryName));
}

template<>
inline std::string absolute<StdFileIO>(std::string &fileName)
{
    return generic::fs::absolute(generic::fs::path(fileName)).string();
}

template<>
inline std::string parent_path<StdFileIO>(std::string &fileName)
{
    return generic::fs::path(fileName).parent_path().string();
}

template<>
inline std::string stem<StdFileIO>(std::string &fileName)
{
    return generic::fs::path(fileName).stem().string();
}

template<>
inline std::string filename<StdFileIO>(std::string &fileName)
{
    return generic::fs::path(fileName).filename().string();
}

template<>
inline typename StdFileIO::Time lastWriteTime<StdFileIO>(const std::string &fileName, std::error_code &errorCode)
{
    return generic::fs::last_write_time(generic::fs::path(fileName), errorCode);
}

typedef fs<StdFileIO> stdfs;

}//namespace io
}//namespace generic

#pragma warning(pop)

#endif //_generic_stdFileIO_h_ 