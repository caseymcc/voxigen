#ifdef LOG_PROCESS_QUEUE
#include <glog/logging.h>
#endif//LOG_PROCESS_QUEUE

namespace voxigen
{

template<typename _Grid>
void ProcessQueue<_Grid>::checkInputThread()
{
#ifndef NDEBUG
    //lets make sure only one thread is changing the input queue
    if(!m_inputThreadIdSet)
    {
        m_inputThreadId=std::this_thread::get_id();
        m_inputThreadIdSet=true;
    }

    assert(std::this_thread::get_id()==m_inputThreadId);
#endif
}

template<typename _Grid>
void ProcessQueue<_Grid>::checkOutputThread()
{
#ifndef NDEBUG
    //lets make sure only one thread is changing the input queue
    if(!m_outputThreadIdSet)
    {
        m_outputThreadId=std::this_thread::get_id();
        m_outputThreadIdSet=true;
    }

    assert(std::this_thread::get_id()==m_outputThreadId);
#endif
}

template<typename _Grid>
bool ProcessQueue<_Grid>::updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk)
{
//    std::shared_ptr<UpdateQueueRequestType> request=std::make_shared<UpdateQueueRequestType>(region, chunk);
    ProcessRequest *request=m_requests.get();

    if(!request)
        return false;

    request->type=Process::Type::UpdateQueue;
    request->priority=Process::Priority::UpdateQueue;

    request->regionIndex=region;
    request->chunkIndex=chunk;

    checkInputThread();
    m_cacheQueue.push_back(request);
//    updateQueue();
    
    return true;
}

template<typename _Grid>
bool ProcessQueue<_Grid>::addGenerateRegion(RegionHandleType *handle, size_t lod)
{
    ProcessRequest *request=m_requests.get();

    if(!request)
    {
#ifdef LOG_PROCESS_QUEUE
        Log::debug("Generate region (%d, %d, %d) failed, no request available",
            chunkHandle->regionIndex().x, chunkHandle->regionIndex().y, chunkHandle->regionIndex().z);
#endif
        return false;
    }

    request->type=Process::Type::GenerateRegion;
    request->priority=Process::Priority::GenerateRegion;

    request->handle.region=handle;
    request->regionIndex=handle->regionIndex();
    request->lod=lod;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("Adding generate region (%d, %d, %d)", request->regionIndex.x, request->regionIndex.y, request->regionIndex.z);
#endif
    checkInputThread();
    m_cacheQueue.push_back(request);

    return true;
}

template<typename _Grid>
bool ProcessQueue<_Grid>::addGenerate(ChunkHandleType *chunkHandle, size_t lod)
{
    ProcessRequest *request=m_requests.get();

    if(!request)
    {
#ifdef LOG_PROCESS_QUEUE
        Log::debug("Generate chunk (%d, %d, %d  %d, %d, %d) failed, no request available",
            chunkHandle->regionIndex().x, chunkHandle->regionIndex().y, chunkHandle->regionIndex().z,
            chunkHandle->chunkIndex().x, chunkHandle->chunkIndex().y, chunkHandle->chunkIndex().z);
#endif
        return false;
    }

    request->type=Process::Type::Generate;
    request->priority=Process::Priority::Generate;

    request->handle.chunk=chunkHandle;
    request->regionIndex=chunkHandle->regionIndex();
    request->chunkIndex=chunkHandle->chunkIndex();
    request->lod=lod;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("Adding generate chunk (%d, %d, %d  %d, %d, %d)",
        request->regionIndex.x, request->regionIndex.y, request->regionIndex.z, 
        request->chunkIndex.x, request->chunkIndex.y, request->chunkIndex.z);
#endif

    checkInputThread();
    m_cacheQueue.push_back(request);
    
    return true;
}

template<typename _Grid>
bool ProcessQueue<_Grid>::addRead(ChunkHandleType *chunkHandle, size_t lod)
{
    ProcessRequest *request=m_requests.get();

    if(!request)
    {
#ifdef LOG_PROCESS_QUEUE
        Log::debug("Read chunk (%d, %d, %d  %d, %d, %d) failed, no request available",
            chunkHandle->regionIndex().x, chunkHandle->regionIndex().y, chunkHandle->regionIndex().z,
            chunkHandle->chunkIndex().x, chunkHandle->chunkIndex().y, chunkHandle->chunkIndex().z);
#endif
        return false;
    }

    request->type=Process::Type::Read;
    request->priority=Process::Priority::Read;

    request->handle.chunk=chunkHandle;
    request->regionIndex=chunkHandle->regionIndex();
    request->chunkIndex=chunkHandle->chunkIndex();

#ifdef LOG_PROCESS_QUEUE
    Log::debug("Adding read chunk (%d, %d, %d  %d, %d, %d)",
        request->regionIndex.x, request->regionIndex.y, request->regionIndex.z,
        request->chunkIndex.x, request->chunkIndex.y, request->chunkIndex.z);
#endif

    checkInputThread();
    m_cacheQueue.push_back(request);

    return true;
}

template<typename _Grid>
bool ProcessQueue<_Grid>::addWrite(ChunkHandleType *chunkHandle)
{
    ProcessRequest *request=m_requests.get();

    if(!request)
    {
#ifdef LOG_PROCESS_QUEUE
        Log::debug("Write chunk (%d, %d, %d  %d, %d, %d) failed, no request available",
            chunkHandle->regionIndex().x, chunkHandle->regionIndex().y, chunkHandle->regionIndex().z,
            chunkHandle->chunkIndex().x, chunkHandle->chunkIndex().y, chunkHandle->chunkIndex().z);
#endif
        return false;
    }

    request->type=Process::Type::Write;
    request->priority=Process::Priority::Write;

    request->handle.chunk=chunkHandle;
    request->regionIndex=chunkHandle->regionIndex();
    request->chunkIndex=chunkHandle->chunkIndex();

#ifdef LOG_PROCESS_QUEUE
    Log::debug("Adding write chunk (%d, %d, %d  %d, %d, %d)",
        request->regionIndex.x, request->regionIndex.y, request->regionIndex.z,
        request->chunkIndex.x, request->chunkIndex.y, request->chunkIndex.z);
#endif

    checkInputThread();
    m_cacheQueue.push_back(request);

    return true;
}

template<typename _Grid>
bool ProcessQueue<_Grid>::addUpdate(ChunkHandleType *chunkHandle)
{
    chunkHandle->setUpdating();
    
    ProcessRequest *request=m_requests.get();

    if(!request)
    {
#ifdef LOG_PROCESS_QUEUE
        Log::debug("Update chunk (%d, %d, %d  %d, %d, %d) failed, no request available",
            chunkHandle->regionIndex().x, chunkHandle->regionIndex().y, chunkHandle->regionIndex().z,
            chunkHandle->chunkIndex().x, chunkHandle->chunkIndex().y, chunkHandle->chunkIndex().z);
#endif
        return false;
    }

    request->type=Process::Type::Update;
    request->priority=Process::Priority::Update;

    request->handle.chunk=chunkHandle;
    request->regionIndex=chunkHandle->regionIndex();
    request->chunkIndex=chunkHandle->chunkIndex();

#ifdef LOG_PROCESS_QUEUE
    Log::debug("Adding update chunk (%d, %d, %d  %d, %d, %d)",
        request->regionIndex.x, request->regionIndex.y, request->regionIndex.z,
        request->chunkIndex.x, request->chunkIndex.y, request->chunkIndex.z);
#endif

    checkInputThread();
    m_cacheQueue.push_back(request);
}

template<typename _Grid>
bool ProcessQueue<_Grid>::addCancel(ChunkHandleType *chunkHandle)
{
    ProcessRequest *request=m_requests.get();

    if(!request)
    {
#ifdef LOG_PROCESS_QUEUE
        Log::debug("Cancel chunk (%d, %d, %d  %d, %d, %d) failed, no request available",
            chunkHandle->regionIndex().x, chunkHandle->regionIndex().y, chunkHandle->regionIndex().z,
            chunkHandle->chunkIndex().x, chunkHandle->chunkIndex().y, chunkHandle->chunkIndex().z);
#endif
        return false;
    }

    request->type=Process::Type::Cancel;
    request->priority=Process::Priority::Cancel;

    request->handle.chunk=chunkHandle;
    request->regionIndex=chunkHandle->regionIndex();
    request->chunkIndex=chunkHandle->chunkIndex();

#ifdef LOG_PROCESS_QUEUE
    Log::debug("Adding cancel chunk (%d, %d, %d  %d, %d, %d)",
        request->regionIndex.x, request->regionIndex.y, request->regionIndex.z,
        request->chunkIndex.x, request->chunkIndex.y, request->chunkIndex.z);
#endif

    checkInputThread();
    m_cacheQueue.push_back(request);
}

template<typename _Grid>
bool ProcessQueue<_Grid>::addRelease(ChunkHandleType *chunkHandle)
{
//    chunkHandle->setAction(HandleAction::Releasing);
//    std::shared_ptr<ReleaseRequestType> request=std::make_shared<ReleaseRequestType>(chunkHandle);
//
//    checkInputThread();
//    m_cacheQueue.push_back(request);
}

template<typename _Grid>
void ProcessQueue<_Grid>::releaseRequest(ProcessRequest *request)
{
    checkInputThread();
    m_requests.release(request);
}

template<typename _Grid>
void ProcessQueue<_Grid>::updateQueue()
{
    if(!m_cacheQueue.empty())
    {
        {//hold lock only as long as required
            std::unique_lock<std::mutex> lock(m_queueMutex);

            m_queue.insert(m_queue.end(), m_cacheQueue.begin(), m_cacheQueue.end());
        }
        m_cacheQueue.resize(0);
        m_checkForUpdates++;
    }
    m_queueEvent.notify_all();
}

template<typename _Grid>
void ProcessQueue<_Grid>::updatePriorityQueue()
{
    RequestQueue localQueue;

    {//hold lock only as long as required
        std::unique_lock<std::mutex> lock(m_queueMutex);

        localQueue.insert(localQueue.end(), m_queue.begin(), m_queue.end());
        m_queue.resize(0);
    }

    for(ProcessRequest *request:localQueue)
    {
        m_priorityQueue.push_back(request);
        std::push_heap(m_priorityQueue.begin(), m_priorityQueue.end(), ProcessCompare<Region, ChunkType>());
    }
}

template<typename _Grid>
std::unique_lock<std::mutex> ProcessQueue<_Grid>::getLock()
{
    std::unique_lock<std::mutex> lock(m_queueMutex);

    return lock;
}

template<typename _Grid>
void ProcessQueue<_Grid>::trigger(std::unique_lock<std::mutex> &lock)
{
    m_queueEvent.notify_all();
}

template<typename _Grid>
void ProcessQueue<_Grid>::wait(std::unique_lock<std::mutex> &lock)
{
    m_queueEvent.wait(lock);
}

template<typename _Grid>
//typename ProcessQueue<_Grid>::SharedChunkHandle ProcessQueue<_Grid>::getNextProcessRequest(std::unique_lock<std::mutex> &lock, Process::Type &type, size_t &data)
typename ProcessQueue<_Grid>::ProcessRequest *ProcessQueue<_Grid>::getNextProcessRequest()
{
    ProcessRequest *request=nullptr;
    RequestQueue cancelRequests;
///    SharedChunkHandle chunkHandle;

    //atomic used for notifying thread of changes
    if(m_checkForUpdates>0)
    {
        updatePriorityQueue();
        m_checkForUpdates=0;
    }

    while(!m_priorityQueue.empty())
    {
//        request=m_queue.top();
//        m_queue.pop_back();
        std::pop_heap(m_priorityQueue.begin(), m_priorityQueue.end(), ProcessCompare<Region, ChunkType>());
        request=m_priorityQueue.back();
        m_priorityQueue.pop_back();

        if(request->type==Process::Type::Cancel)
        {
            cancelRequests.push_back(request);
            continue;
        }

        switch(request->type)
        {
        case Process::Type::UpdateQueue:
            {
//                UpdateQueueRequest *updateRequest=static_cast<UpdateQueueRequest *>(request.get());
                //position changed so update queue priority
                ProcessCompare<Region, ChunkType>::currentRegion=request->regionIndex;
                ProcessCompare<Region, ChunkType>::currentChunk=request->chunkIndex;

#ifdef LOG_PROCESS_QUEUE
                Log::debug("Updating queue pos (%d, %d, %d  %d, %d, %d)",
                    request->regionIndex.x, request->regionIndex.y, request->regionIndex.z, request->chunkIndex.x, request->chunkIndex.y, request->chunkIndex.z);
#endif//LOG_PROCESS_QUEUE
                sortPriorityQueue();

                m_completedQueueCache.push_back(request);
                continue;
            }
            break;
        }
        break;
    }

    //clear all request request
    if(!cancelRequests.empty())
        removeRequests(cancelRequests);

#ifdef LOG_PROCESS_QUEUE
//    if(chunkHandle)
//    {
//        glm::ivec3 regionIndex=chunkHandle->regionIndex();
//        glm::ivec3 chunkIndex=chunkHandle->chunkIndex();
//
//        Log::debug("Processing chunk (%d, %d, %d  %d, %d, %d) distance %d",
//            regionIndex.x, regionIndex.y, regionIndex.z, chunkIndex.x, chunkIndex.y, chunkIndex.z, request->distance);
//    }
#endif
//    return chunkHandle;
    return request;
}

template<typename _Grid>
void ProcessQueue<_Grid>::removeRequests(RequestQueue &cancelRequests)
{
    size_t removed=0;
//    std::vector<SharedChunkHandle> chunkHandles;
//
//    //lock all chunk handles for request that we are about to delete
//    for(SharedRequest &request:cancelRequests)
//    {
//        CancelRequestType *cancelRequest=(CancelRequestType *)request.get();
//        SharedChunkHandle lockedChunkHandle=cancelRequest->chunkHandle.lock();
//
//        if(lockedChunkHandle)
//            chunkHandles.push_back(lockedChunkHandle);
//    }
//
////    for(auto iter=m_priorityQueue.begin(); iter!=m_priorityQueue.end();)
//    std::for_each(m_priorityQueue.begin(), m_priorityQueue.end(), [&](SharedRequest &request)
//    {
////        SharedRequest &request=*iter;
//        SharedChunkHandle requestChunkHandle;
//
//        switch(request->type)
//        {
//        case Process::Type::UpdateQueue:
//            break;
//        case Process::Type::Generate:
//            {
//                GenerateRequestType *generateRequest=(GenerateRequestType *)request.get();
//
//                requestChunkHandle=generateRequest->chunkHandle.lock();
//            }
//            break;
//        case Process::Type::Read:
//            {
//                ReadRequestType *readRequest=(ReadRequestType *)request.get();
//
//                requestChunkHandle=readRequest->chunkHandle.lock();
//            }
//            break;
//        case Process::Type::Update:
//            {
//                UpdateRequestType *updateRequest=(UpdateRequestType *)request.get();
//
//                requestChunkHandle=updateRequest->chunkHandle.lock();
//            }
//            break;
//        case Process::Type::Release:
//            {
//                ReleaseRequestType *releaseRequest=(ReleaseRequestType *)request.get();
//
//                requestChunkHandle=releaseRequest->chunkHandle.lock();
//            }
//            break;
//        }
//
//        //handle dropped so remove request
//        if(!requestChunkHandle)
//        {
//            request.reset();
//            removed++;
//        }
//
//        for(size_t i=0; i<chunkHandles.size(); ++i)
//        {
//            SharedChunkHandle &chunkHandle=chunkHandles[i];
//
//            if(requestChunkHandle==chunkHandle)
//            {
//                request.reset();
//                chunkHandles[i]=chunkHandles.back();
//                chunkHandles.pop_back();
//                removed++;
//                break;
//            }
//        }
//
//    });

    //looping priority first as the cancelRequest vector should be smaller and shrinking to 0
    for(size_t i=0; i<m_priorityQueue.size(); ++i)
    {
        ProcessRequest *request=m_priorityQueue[i];

        for(size_t j=0; j<cancelRequests.size(); ++j)
        {
            if(cancelRequests[j]==request)
            {
                //set request as completed
                m_priorityQueue[i]->result=Process::Result::Canceled;
                m_completedQueueCache.push_back(m_priorityQueue[i]);
                m_priorityQueue[i]=nullptr;
                cancelRequests[j]=cancelRequests.back();
                cancelRequests.pop_back();
                removed++;
                break;
            }
        }
    }

    //reset m_priorityQueue by removing the nullptrs and reordering
    if(removed>0)
    {
        for(size_t i=0; i+1<m_priorityQueue.size(); ++i)
        {
            if(!m_priorityQueue[i])
            {
                while((i+1<m_priorityQueue.size()) && !m_priorityQueue.back())
                    m_priorityQueue.pop_back();

                m_priorityQueue[i]=m_priorityQueue.back();
                m_priorityQueue.pop_back();
            }
        }

        while(!m_priorityQueue.empty() && !m_priorityQueue.back())
            m_priorityQueue.pop_back();

        sortPriorityQueue();
    }
}

template<typename _Grid>
void ProcessQueue<_Grid>::addCompletedRequest(ProcessRequest *request)
{
    m_completedQueueCache.push_back(request);
}

template<typename _Grid>
void ProcessQueue<_Grid>::updateCompletedQueue()
{
    {
        std::unique_lock<std::mutex> lock(m_completedMutex);

        m_completedQueue.insert(m_completedQueue.end(), m_completedQueueCache.begin(), m_completedQueueCache.end());
    }
    m_completedQueueCache.resize(0);
}

template<typename _Grid>
void ProcessQueue<_Grid>::getCompletedQueue(RequestQueue &completedQueue)
{
    size_t startIndex=completedQueue.size();
    {
        std::unique_lock<std::mutex> lock(m_completedMutex);

        completedQueue.insert(completedQueue.end(), m_completedQueue.begin(), m_completedQueue.end());
        m_completedQueue.resize(0);
    }

//    //lets clear chunk action
//    for(size_t index=startIndex; index<completedQueue.size(); ++index)
//    {
//        SharedRequest &request=completedQueue[index];
//        auto chunkHandle=request->getChunkHandle();
//
////        if(chunkHandle)
////            chunkHandle->setIdle();
//    }
}

template<typename _Grid>
bool ProcessQueue<_Grid>::isCompletedQueueEmpty()
{
    return m_completedQueue.empty();
}

//template<typename _Grid>
//void ProcessQueue<_Grid>::updatePriority()
//{
//    std::unique_lock<std::mutex> lock(m_queueMutex);
//
//    std::make_heap(m_priorityQueue.begin(), m_priorityQueue.end(), ProcessCompare<ChunkType>());
//}

template<typename _Grid>
void ProcessQueue<_Grid>::sortPriorityQueue()
{
    std::make_heap(m_priorityQueue.begin(), m_priorityQueue.end(), ProcessCompare<Region, ChunkType>());
}


}//namespace voxigen

