#ifndef _generic_watchFiles_h_
#define _generic_watchFiles_h_

#include "generic/generic_export.h"
#include "generic/stdFileIO.h"

#include <string>
#include <vector>
#include <memory>

namespace generic
{
namespace io
{
    struct WatchFilesPrivate;
    class WatchFiles
    {
    public:
        WatchFiles();
        ~WatchFiles();

        bool add(const char *file);
        bool add(const std::string &file);

        void remove(const char *file);
        void remove(const std::string &file);

        bool check(char ** const changedFiles, size_t *sizes, size_t &count);
        std::vector<std::string> check();

    private:
        std::unique_ptr<WatchFilesPrivate> m_private;
    };

    inline bool WatchFiles::add(const std::string &file)
    { return add(file.c_str()); }

    inline void WatchFiles::remove(const std::string &file)
    { remove(file.c_str()); }

    inline std::vector<std::string> WatchFiles::check()
    {
        std::vector<std::string> updatedFiles;
        std::vector<char *> updatedFilesPtr;
        std::vector<size_t> sizes;
        size_t count;

        while(true)
        {
            count=updatedFiles.size();
            
            check(nullptr, nullptr, count);

            if(count==0)//no updates
                break;

            updatedFiles.resize(count);
            updatedFilesPtr.resize(count);
            sizes.resize(count);

            check(nullptr, sizes.data(), count);

            if(count != sizes.size())
                continue;

            for(size_t i=0; i<sizes.size(); ++i)
            {
                updatedFiles[i].resize(sizes[i]-1); //string will add extra for null terminator
                updatedFilesPtr[i]=const_cast<char *>(updatedFiles[i].data());
            }

            if(check(updatedFilesPtr.data(), sizes.data(), count))
                break;
        }

        return updatedFiles;
    }
}//namespace io
}//namespace generic

#endif //_generic_watchFiles_h_ 