#ifndef _generic_fileIO_h_
#define _generic_fileIO_h_

#include "generic/generic_export.h"

#include <string>
#include <system_error>

namespace generic
{
namespace io
{
    template<typename _FileIO>
    typename _FileIO::Type *open(std::string filename, std::string mode, void *userData=nullptr)
    {
        return (typename _FileIO::Type *)nullptr;
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

    template<typename _FileIO>
    bool exists(const std::string &fileName)
    {
        return false;
    }

    template<typename _FileIO>
    bool create_directory(const std::string &directoryName)
    {
        return false;
    }

    template<typename _FileIO>
    std::string absolute(std::string &fileName)
    {
        return std::string();
    }

    template<typename _FileIO>
    std::string parent_path(std::string &fileName)
    {
        return std::string();
    }

    template<typename _FileIO>
    std::string stem(std::string &fileName)
    {
        return std::string();
    }

    template<typename _FileIO>
    std::string filename(std::string &fileName)
    {
        return std::string();
    }

    template<typename _FileIO>
    typename _FileIO::Time lastWriteTime(const std::string &fileName, std::error_code &errorCode)
    {
        return _FileIO::Time();
    }

    GENERIC_EXPORT size_t getExtension(const char *fileName, char *ext, size_t extSize);

    inline std::string getExtension(std::string fileName)
    {
        //guess 4 and less plus "."
        std::string ext(5, 0);

        size_t extRead=generic::io::getExtension(fileName.c_str(), (char *)ext.data(), ext.size());

        if(extRead>5)
        {
            //if it is larger than guessed, resize and read again
            ext.resize(extRead);
            extRead=generic::io::getExtension(fileName.c_str(), (char *)ext.data(), ext.size());
        }
        else
            ext.resize(extRead);

        return ext;
    }

    template<typename _FileIO>
    struct fs
    {
        typedef typename _FileIO::Type Type;

        static typename _FileIO::Type *open(std::string filename, std::string mode, void *userData=nullptr)
        {   return io::open<_FileIO>(filename, mode, userData);}
        
        static void close(typename _FileIO::Type *type)
        {   io::close<_FileIO>(type); }

        static size_t read(void *ptr, size_t size, size_t count, typename _FileIO::Type *type)
        {
            return io::read<_FileIO>(ptr, size, count, type);
        }

        static size_t write(void *ptr, size_t size, size_t count, typename _FileIO::Type *type)
        {
            return io::write<_FileIO>(ptr, size, count, type);
        }

        static size_t size(typename _FileIO::Type *type)
        {
            return io::size<_FileIO>(type);
        }

        static bool exists(const std::string &fileName)
        {
            return io::exists<_FileIO>(fileName);
        }

        static bool create_directory(const std::string &directoryName)
        {
            return io::create_directory<_FileIO>(directoryName);
        }

        static std::string absolute(std::string &fileName)
        {
            return io::absolute<_FileIO>(fileName);
        }

        static std::string parent_path(std::string &fileName)
        {
            return io::parent_path<_FileIO>(fileName);
        }

        static std::string stem(std::string &fileName)
        {
            return io::stem<_FileIO>(fileName);
        }

        static std::string filename(std::string &fileName)
        {
            return io::filename<_FileIO>(fileName);
        }

        static typename _FileIO::Time lastWriteTime(const std::string &fileName, std::error_code &errorCode)
        {
            return io::lastWriteTime<_FileIO>(fileName, errorCode);
        }
    };
    
}//namespace io
}//namespace generic

#endif //_generic_fileIO_h_ 