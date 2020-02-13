#include "generic/watchFiles.h"
#include "generic/filesystem.h"
#include "generic/stdFileIO.h"

#ifdef _WIN32
#include <windows.h>
#else
#endif

#include <cassert>

namespace generic
{
namespace io
{

struct FileInfo
{
    FileInfo(const std::string &name, StdFileIO::Time lastWriteTime):name(name), lastWriteTime(lastWriteTime) {}

    std::string name;
    StdFileIO::Time lastWriteTime;
};

struct DirectoryInfo
{
    DirectoryInfo() {}
    DirectoryInfo(const std::string &name, std::vector<FileInfo> files):name(name), files(files) {}

    std::string name;
    std::vector<FileInfo> files;
};

struct WatchFilesPrivate
{
    std::vector<DirectoryInfo> directories;
#ifdef _WIN32
    std::vector<HANDLE> handles;
    std::vector<std::string> updatedFiles;
#else
#endif
};

WatchFiles::WatchFiles()
{
    m_private.reset(new WatchFilesPrivate());
}

WatchFiles::~WatchFiles()
{
}

#ifdef _WIN32
bool WatchFiles::add(const char *filePath)
{
    std::string fileAbsolutePath=generic::io::absolute<generic::io::StdFileIO>(std::string(filePath));
    std::string fileDirectory=generic::io::parent_path<generic::io::StdFileIO>(std::string(fileAbsolutePath));
    std::string fileName=generic::io::filename<generic::io::StdFileIO>(std::string(fileAbsolutePath));

    for(auto iter=m_private->directories.begin(); iter!=m_private->directories.end(); ++iter)
    {
        DirectoryInfo &info=*iter;

        if(info.name == fileDirectory)
        {
            bool found=false;
            for(auto fileIter=info.files.begin(); fileIter!=info.files.end(); ++fileIter)
            {
                if(fileIter->name == fileName)
                {
                    found=true;
                    break;
                    
                }
            }

            if(!found)
            {
                std::error_code errorCode;
                auto lastWrite=lastWriteTime<StdFileIO>(fileAbsolutePath, errorCode);

                if(errorCode)
                    return false;
                info.files.push_back({fileName, lastWrite});
            }
            return true;
        }
    }

    //add directory to notification system
    DirectoryInfo dirInfo;

    HANDLE changeHandle=FindFirstChangeNotificationA((LPCSTR)fileDirectory.c_str(), false, FILE_NOTIFY_CHANGE_FILE_NAME);
    std::error_code errorCode;
    auto lastWrite=lastWriteTime<StdFileIO>(fileAbsolutePath, errorCode);

    if(errorCode)
        return false;

    dirInfo.name=fileDirectory;
    dirInfo.files.emplace_back(fileName, lastWrite);

    m_private->directories.emplace_back(dirInfo);
    m_private->handles.push_back(changeHandle);
    
    return true;
}

void WatchFiles::remove(const char *filePath)
{
    std::string fileAbsolutePath=generic::io::absolute<generic::io::StdFileIO>(std::string(filePath));
    std::string fileDirectory=generic::io::parent_path<generic::io::StdFileIO>(fileAbsolutePath);
    std::string fileName=generic::io::filename<generic::io::StdFileIO>(fileAbsolutePath);

    for(size_t index=0; index<m_private->directories.size(); ++index)
    {
        DirectoryInfo &info=m_private->directories[index];

        if(info.name == fileDirectory)
        {
            for(auto fileIter=info.files.begin(); fileIter!=info.files.end(); ++fileIter)
            {
                if(fileIter->name == fileName)
                {
                    info.files.erase(fileIter);
                    break;
                }
            }

            if(info.files.empty())
            {
                //not listening to any other files, so lets close
                FindCloseChangeNotification(m_private->handles[index]);

                m_private->directories.erase(m_private->directories.begin()+index);
                m_private->handles.erase(m_private->handles.begin()+index);
            }

            return;
        }
    }
}

bool WatchFiles::check(char ** const changedFiles, size_t *sizes, size_t &count)
{
    DWORD status;
    auto &updatedFiles=m_private->updatedFiles;

    while(true)
    {
        status=WaitForMultipleObjects((DWORD)m_private->handles.size(), m_private->handles.data(), FALSE, 0);

        if(status == WAIT_TIMEOUT)
            break;

        size_t index=status-WAIT_OBJECT_0;

        if((index<0) || (index >= m_private->directories.size()))
            break;
        
        DirectoryInfo &info=m_private->directories[index];

        for(auto iter=info.files.begin(); iter!=info.files.end(); ++iter)
        {
            std::string fileName=info.name+"/"+iter->name;
            std::error_code errorCode;
            auto lastWrite=lastWriteTime<StdFileIO>(fileName, errorCode);
            
            bool update=false;
            
            if(errorCode)//something wrong lets say something changed
                update=true; //not updating time so may cause multiple updates is notification comes again
            else
            {
                if(lastWrite!=iter->lastWriteTime)
                {
                    iter->lastWriteTime=lastWrite;
                    update=true;
                }
            }

            if(update)
            {
                auto updateIter=std::find(updatedFiles.begin(), updatedFiles.end(), fileName);

                if(updateIter==updatedFiles.end())
                    updatedFiles.push_back(fileName);
            }
        }

        if(FindNextChangeNotification(m_private->handles[index])==FALSE)
        {
            //failed to set notification, remove the directory watcher
            FindCloseChangeNotification(m_private->handles[index]);

            m_private->directories.erase(m_private->directories.begin()+index);
            m_private->handles.erase(m_private->handles.begin()+index);

            break;
        }
    }

    if(!sizes)
    {
        count=updatedFiles.size();
        return false;
    }

    if(count<updatedFiles.size())
    {
        count=updatedFiles.size();
        return false;
    }

    bool sizeMatches=true;
    
    for(size_t index=0; index<updatedFiles.size(); ++index)
    {
        if(sizes[index]<=updatedFiles[index].size()) //need space for null terminator, cant be equal
        {
            sizeMatches=false;
            break;
        }
    }

    if((!changedFiles)||(!sizeMatches))
    {
        for(size_t index=0; index<updatedFiles.size(); ++index)
            sizes[index]=updatedFiles[index].size()+1;//need space for null terminator

        for(size_t index=updatedFiles.size(); index<count; ++index)
            sizes[index]=0;

        return false;
    }

    for(size_t index=0; index<updatedFiles.size(); ++index)
        strncpy(changedFiles[index], updatedFiles[index].data(), updatedFiles[index].size());

    updatedFiles.clear();
    return true;
}

#else
void WatchFiles::add(char *file)
{
    assert(false);
}

void WatchFiles::remove(char *file)
{
    assert(false);
}
#endif
    
}//namespace io
}//namespace generic