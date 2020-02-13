#ifndef _voxigen_dataHandler_h_
#define _voxigen_dataHandler_h_

#include <mutex>
#include <memory>
#include <unordered_map>

namespace voxigen
{

template<typename _HashType, typename _DataHandle, typename _Data>
class DataHandler
{
public:
    typedef _HashType HashType;

    typedef _Data Data;
    typedef std::unique_ptr<Data> UniqueData;

//    typedef _DataHandle<_Data> Handle;
    typedef _DataHandle DataHandle;

    typedef std::shared_ptr<DataHandle> SharedDataHandle;
    typedef std::unordered_map<_HashType, SharedDataHandle> SharedDataHandleMap;

    typedef std::weak_ptr<DataHandle> WeakDataHandle;
    typedef std::unordered_map<_HashType, WeakDataHandle> WeakDataHandleMap;
    

    DataHandler();
    virtual ~DataHandler(){}

    size_t handlesInUse();

    SharedDataHandle getDataHandle(HashType hash);
    void removeHandle(DataHandle *dataHandle);

protected:
    virtual DataHandle *newHandle(HashType hash)=0;

    void checkThreadSafety();

//Data handles
//    std::mutex m_dataMutex;
#ifndef NDEBUG
    std::thread::id m_threadId;
    bool m_threadIdSet;
#endif

    SharedDataHandleMap m_dataHandles;
    WeakDataHandleMap m_weakDataHandles;
};

template<typename _HashType, typename _DataHandle, typename _Data>
DataHandler<_HashType, _DataHandle, _Data>::DataHandler()
#ifndef NDEBUG
    :m_threadIdSet(false)
#endif
{
}

template<typename _HashType, typename _DataHandle, typename _Data>
size_t DataHandler<_HashType, _DataHandle, _Data>::handlesInUse()
{
//    std::unique_lock<std::mutex> lock(m_dataMutex);
    checkThreadSafety();

    size_t count=0;
    for(auto iter:m_weakDataHandles)
    {
        if(iter.second.lock())
            count++;
    }

    return count;
}

template<typename _HashType, typename _DataHandle, typename _Data>
typename DataHandler<_HashType, _DataHandle, _Data>::SharedDataHandle DataHandler<_HashType, _DataHandle, _Data>::getDataHandle(HashType hash)
{
//    std::unique_lock<std::mutex> lock(m_dataMutex);
    checkThreadSafety();

    auto iter=m_weakDataHandles.find(hash);
    SharedDataHandle returnHandle;

    if(iter!=m_weakDataHandles.end())
    {
        if(!iter->second.expired())
        {
            returnHandle=iter->second.lock();

            if(returnHandle)
                return returnHandle;//we already have it and somebody else has it as well
        }
    }

    //see if we already know about the data
    auto handleIter=m_dataHandles.find(hash);

    if(handleIter==m_dataHandles.end())
    {
        //we dont know about this one, create a handle
        SharedDataHandle handle;
        DataHandle *dataHandle=newHandle(hash);

        //ceate local handle for data storage
        handle.reset(dataHandle);
        //Create shared handle to notify handler when it is no longer in use
        returnHandle.reset(dataHandle, std::bind(&DataHandler<_HashType, _DataHandle, _Data>::removeHandle, this, std::placeholders::_1));

        m_dataHandles[hash]=handle;
    }
    else
    {
        DataHandle *dataHandle=handleIter->second.get();

        //Create shared handle to notify handler when it is no longer in use
        returnHandle.reset(dataHandle, std::bind(&DataHandler<_HashType, _DataHandle, _Data>::removeHandle, this, std::placeholders::_1));
    }

    m_weakDataHandles[hash]=returnHandle;

    if(!returnHandle)
        return returnHandle;
    return returnHandle;
}

template<typename _HashType, typename _DataHandle, typename _Data>
void DataHandler<_HashType, _DataHandle, _Data>::removeHandle(DataHandle *dataHandle)
{
//    std::unique_lock<std::mutex> lock(m_dataMutex);
    checkThreadSafety();

    auto iter=m_weakDataHandles.find(dataHandle->hash());

    if(iter!=m_weakDataHandles.end())
        m_weakDataHandles.erase(iter);

//    //we are not releasing the handle here as we hold it in a different map
//    //but we want to release any data attached to it as no one is using it
//    dataHandle->release();
}

template<typename _HashType, typename _DataHandle, typename _Data>
void DataHandler<_HashType, _DataHandle, _Data>::checkThreadSafety()
{
#ifndef NDEBUG
    //lets make sure only one thread is changing the input queue
    if(!m_threadIdSet)
    {
        m_threadId=std::this_thread::get_id();
        m_threadIdSet=true;
    }

    assert(std::this_thread::get_id()==m_threadId);
#endif
}

} //namespace voxigen

#endif //_voxigen_dataHandler_h_