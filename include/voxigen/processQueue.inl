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
void ProcessQueue<_Grid>::updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk)
{
    std::shared_ptr<UpdateQueueRequestType> request=std::make_shared<UpdateQueueRequestType>(region, chunk);

    m_cacheQueue.push_back(request);
    
    updateQueue();
}

template<typename _Grid>
void ProcessQueue<_Grid>::addGenerateRegion(SharedRegionHandle handle, size_t lod)
{
    std::shared_ptr<GenerateRegionRequestType> request=std::make_shared<GenerateRegionRequestType>(handle, lod);

#ifdef LOG_PROCESS_QUEUE
    if(handle)
    {
        glm::ivec3 regionIndex=handle->regionIndex();

        Log::debug("Adding generate region (%d, %d, %d)", regionIndex.x, regionIndex.y, regionIndex.z);
    }
#endif
    checkInputThread();
    m_cacheQueue.push_back(request);
}

template<typename _Grid>
void ProcessQueue<_Grid>::addGenerate(SharedChunkHandle chunkHandle, size_t lod)
{
    std::shared_ptr<GenerateRequestType> request=std::make_shared<GenerateRequestType>(chunkHandle, lod);

#ifdef LOG_PROCESS_QUEUE
    if(chunkHandle)
    {
        glm::ivec3 regionIndex=chunkHandle->regionIndex();
        glm::ivec3 chunkIndex=chunkHandle->chunkIndex();

        Log::debug("Adding generate chunk (%d, %d, %d  %d, %d, %d)",
            regionIndex.x, regionIndex.y, regionIndex.z, chunkIndex.x, chunkIndex.y, chunkIndex.z);
    }
#endif
    checkInputThread();
    m_cacheQueue.push_back(request);
}

template<typename _Grid>
void ProcessQueue<_Grid>::addRead(SharedChunkHandle chunkHandle, size_t lod)
{
    std::shared_ptr<ReadRequestType> request=std::make_shared<ReadRequestType>(chunkHandle, lod);

    checkInputThread();
    m_cacheQueue.push_back(request);
}

template<typename _Grid>
void ProcessQueue<_Grid>::addWrite(SharedChunkHandle chunkHandle)
{
    std::shared_ptr<WriteRequestType> request=std::make_shared<WriteRequestType>(chunkHandle);

    checkInputThread();
    m_cacheQueue.push_back(request);
}

template<typename _Grid>
void ProcessQueue<_Grid>::addUpdate(SharedChunkHandle chunkHandle)
{
    chunkHandle->setUpdating();
    std::shared_ptr<UpdateRequestType> request=std::make_shared<UpdateRequestType>(chunkHandle);

    checkInputThread();
    m_cacheQueue.push_back(request);
}

template<typename _Grid>
void ProcessQueue<_Grid>::addCancel(SharedChunkHandle chunkHandle)
{
//    chunkHandle->setReleasing();
    std::shared_ptr<CancelRequestType> request=std::make_shared<CancelRequestType>(chunkHandle);

    checkInputThread();
    m_cacheQueue.push_back(request);
}

template<typename _Grid>
void ProcessQueue<_Grid>::addRelease(SharedChunkHandle chunkHandle)
{
    chunkHandle->setAction(HandleAction::Releasing);
    std::shared_ptr<ReleaseRequestType> request=std::make_shared<ReleaseRequestType>(chunkHandle);

    checkInputThread();
    m_cacheQueue.push_back(request);
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

    for(SharedRequest &request:localQueue)
    {
        m_priorityQueue.push_back(request);
        std::push_heap(m_priorityQueue.begin(), m_priorityQueue.end(), ProcessCompare<ChunkType>());
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
typename ProcessQueue<_Grid>::SharedRequest ProcessQueue<_Grid>::getNextProcessRequest()
{
    SharedRequest request;
    RequestQueue cancelRequests;
///    SharedChunkHandle chunkHandle;

    if(m_checkForUpdates>0)
    {
        updatePriorityQueue();
        m_checkForUpdates=0;
    }

    while(!m_priorityQueue.empty())
    {
//        request=m_queue.top();
//        m_queue.pop_back();
        std::pop_heap(m_priorityQueue.begin(), m_priorityQueue.end(), ProcessCompare<ChunkType>());
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
                UpdateQueueRequest *updateRequest=static_cast<UpdateQueueRequest *>(request.get());

                //position changed so update queue priority
                ProcessCompare<ChunkType>::currentRegion=updateRequest->regionIndex;
                ProcessCompare<ChunkType>::currentChunk=updateRequest->chunkIndex;

#ifdef LOG_PROCESS_QUEUE
                Log::debug("Updating queue pos (%d, %d, %d  %d, %d, %d)",
                    updateRequest->regionIndex.x, updateRequest->regionIndex.y, updateRequest->regionIndex.z, updateRequest->chunkIndex.x, updateRequest->chunkIndex.y, updateRequest->chunkIndex.z);
#endif//LOG_PROCESS_QUEUE
                std::make_heap(m_priorityQueue.begin(), m_priorityQueue.end(), ProcessCompare<ChunkType>());

                continue;
            }
            break;
//        case Process::Type::Generate:
//            {
//                GenerateRequest<ChunkType> *generateRequest=(GenerateRequest<ChunkType> *)request.get();
//
//                chunkHandle=generateRequest->chunkHandle.lock();
//                data=generateRequest->lod;
//            }
//            break;
//        case Process::Type::Read:
//            {
//                ReadRequest<ChunkType> *readRequest=(ReadRequest<ChunkType> *)request.get();
//
//                chunkHandle=readRequest->chunkHandle.lock();
//                data=readRequest->lod;
//            }
//            break;
//        case Process::Type::Write:
//            {
//                WriteRequest<ChunkType> *writeRequest=(WriteRequest<ChunkType> *)request.get();
//
//                chunkHandle=writeRequest->chunkHandle;
//            }
//            break;
//        case Process::Type::Update:
//            {
//                UpdateRequestType *updateRequest=(UpdateRequestType *)request.get();
//
//                chunkHandle=updateRequest->chunkHandle.lock();
//            }
//            break;
//        case Process::Type::Release:
//            {
//                ReleaseRequestType *releaseRequest=(ReleaseRequestType *)request.get();
//
//                chunkHandle=releaseRequest->chunkHandle.lock();
//            }
//            break;
        }

        break;
    }

    //clear all request request
    if(!cancelRequests.empty())
        removeRequests(cancelRequests);

#ifdef LOG_PROCESS_QUEUE
    if(chunkHandle)
    {
        glm::ivec3 regionIndex=chunkHandle->regionIndex();
        glm::ivec3 chunkIndex=chunkHandle->chunkIndex();

        Log::debug("Processing chunk (%d, %d, %d  %d, %d, %d) distance %d",
            regionIndex.x, regionIndex.y, regionIndex.z, chunkIndex.x, chunkIndex.y, chunkIndex.z, request->distance);
    }
#endif

//    return chunkHandle;
    return request;
}

template<typename _Grid>
void ProcessQueue<_Grid>::removeRequests(RequestQueue &cancelRequests)
{
    size_t removed=0;
    std::vector<SharedChunkHandle> chunkHandles;

    for(SharedRequest &request:cancelRequests)
    {
        CancelRequestType *cancelRequest=(CancelRequestType *)request.get();
        SharedChunkHandle lockedChunkHandle=cancelRequest->chunkHandle.lock();

        if(lockedChunkHandle)
            chunkHandles.push_back(lockedChunkHandle);
    }

//    for(auto iter=m_priorityQueue.begin(); iter!=m_priorityQueue.end();)
    std::for_each(m_priorityQueue.begin(), m_priorityQueue.end(), [&](SharedRequest &request)
    {
//        SharedRequest &request=*iter;
        SharedChunkHandle requestChunkHandle;

        switch(request->type)
        {
        case Process::Type::UpdateQueue:
            break;
        case Process::Type::Generate:
            {
            GenerateRequestType *generateRequest=(GenerateRequestType *)request.get();

                requestChunkHandle=generateRequest->chunkHandle.lock();
            }
            break;
        case Process::Type::Read:
            {
                ReadRequestType *readRequest=(ReadRequestType *)request.get();

                requestChunkHandle=readRequest->chunkHandle.lock();
            }
            break;
        case Process::Type::Update:
            {
                UpdateRequestType *updateRequest=(UpdateRequestType *)request.get();

                requestChunkHandle=updateRequest->chunkHandle.lock();
            }
            break;
        case Process::Type::Release:
            {
                ReleaseRequestType *releaseRequest=(ReleaseRequestType *)request.get();

                requestChunkHandle=releaseRequest->chunkHandle.lock();
            }
            break;
        }

        //handle dropped so remove request
        if(!requestChunkHandle)
        {
            request.reset();
            removed++;
        }

        for(size_t i=0; i<chunkHandles.size(); ++i)
        {
            SharedChunkHandle &chunkHandle=chunkHandles[i];

            if(requestChunkHandle==chunkHandle)
            {
                request.reset();
                chunkHandles[i]=chunkHandles.back();
                chunkHandles.pop_back();
                removed++;
                break;
            }
        }

    });

    if(removed>0)
    {
        for(size_t i=0; i+1<m_priorityQueue.size(); ++i)
        {
            SharedRequest &request=m_priorityQueue[i];

            if(!request)
            {
                while((i+1<m_priorityQueue.size()) && !m_priorityQueue.back())
                    m_priorityQueue.pop_back();

                m_priorityQueue[i]=m_priorityQueue.back();
                m_priorityQueue.pop_back();
            }
        }

        while(!m_priorityQueue.empty() && !m_priorityQueue.back())
            m_priorityQueue.pop_back();

        std::make_heap(m_priorityQueue.begin(), m_priorityQueue.end(), ProcessCompare<ChunkType>());
    }
}

template<typename _Grid>
void ProcessQueue<_Grid>::addCompletedRequest(SharedRequest request)
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

}//namespace voxigen

