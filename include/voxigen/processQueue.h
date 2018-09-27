#ifndef _voxigen_processQueue_h_
#define _voxigen_processQueue_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/defines.h"
#include "voxigen/updateQueue.h"
#include "voxigen/chunkHandle.h"

#include <memory>
#include <thread>
#include <queue>
#include <mutex>

namespace voxigen
{

namespace Process
{
    enum Type
    {
        UpdateQueue,
        Generate,
        Read,
        Write,
        Update,
        Cancel,
        Release
    };
}

template<typename _Chunk>
struct ProcessRequest
{
    typedef _Chunk ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandleType;
    typedef std::weak_ptr<ChunkHandleType> WeakChunkHandleType;

    typedef ProcessRequest<ChunkType> ProcessRequestType;

    ProcessRequest(Process::Type type, unsigned int priority, glm::ivec3 regionIndex, glm::ivec3 chunkIndex):type(type), priority(priority), regionIndex(regionIndex), chunkIndex(chunkIndex)
    {
#ifdef LOG_PROCESS_QUEUE
        distance=-1.0f;
#endif
    }
    virtual ~ProcessRequest() {}

    bool operator<(const ProcessRequest &rhs) const { return priority<rhs.priority; }

    virtual SharedChunkHandleType getChunkHandle()=0;

    Process::Type type;
    unsigned int priority;

#ifdef LOG_PROCESS_QUEUE
    float distance;
#endif

    glm::ivec3 regionIndex;
    glm::ivec3 chunkIndex;
};

template<class _Chunk>
using SharedProcessRequest=std::shared_ptr<ProcessRequest<_Chunk>>;

template<typename _Chunk>
struct UpdateQueueRequest:public ProcessRequest<_Chunk>
{
    UpdateQueueRequest(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex):ProcessRequest<_Chunk>(Process::Type::UpdateQueue, 20, regionIndex, chunkIndex){}

    virtual SharedChunkHandleType getChunkHandle() { return SharedChunkHandleType(); }
};

template<typename _Chunk>
struct GenerateRequest:public ProcessRequest<_Chunk>
{
    GenerateRequest(SharedChunkHandleType chunkHandle, size_t lod):ProcessRequest<_Chunk>(Process::Type::Generate, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod){}
    GenerateRequest(unsigned int priority, SharedChunkHandleType chunkHandle, size_t lod):ProcessRequest<_Chunk>(Type::Generate, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle) lod(lod) {}

    SharedChunkHandleType getChunkHandle() override{ return chunkHandle.lock(); }

    WeakChunkHandleType chunkHandle;
    size_t lod;
};

template<typename _Chunk>
struct ReadRequest:public ProcessRequest<_Chunk>
{
    ReadRequest(SharedChunkHandleType chunkHandle, size_t lod):ProcessRequest<_Chunk>(Process::Type::Read, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod) {}
    ReadRequest(unsigned int priority, SharedChunkHandleType chunkHandle, size_t lod):ProcessRequest<_Chunk>(Type::Read, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle), lod(lod) {}

    SharedChunkHandleType getChunkHandle() override{ return chunkHandle.lock(); }

    WeakChunkHandleType chunkHandle;
    size_t lod;
};

template<typename _Chunk>
struct WriteRequest:public ProcessRequest<_Chunk>
{
    WriteRequest(SharedChunkHandleType chunkHandle):ProcessRequest<_Chunk>(Process::Type::Write, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle){}
    WriteRequest(unsigned int priority, SharedChunkHandleType chunkHandle):ProcessRequest<_Chunk>(Type::Write, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()){}

    SharedChunkHandleType getChunkHandle() override { return chunkHandle; }

    SharedChunkHandleType chunkHandle;
};

template<typename _Chunk>
struct UpdateRequest:public ProcessRequest<_Chunk>
{
    UpdateRequest(SharedChunkHandleType chunkHandle):ProcessRequest<_Chunk>(Process::Type::Update, 100, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle){}
    UpdateRequest(unsigned int priority, SharedChunkHandleType chunkHandle):ProcessRequest<_Chunk>(Type::Update, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle){}

    SharedChunkHandleType getChunkHandle() override { return chunkHandle.lock(); }

    WeakChunkHandleType chunkHandle;
};

template<typename _Chunk>
struct CancelRequest:public ProcessRequest<_Chunk>
{
    CancelRequest(SharedChunkHandleType chunkHandle):ProcessRequest<_Chunk>(Process::Type::Cancel, 9, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle) {}

    SharedChunkHandleType getChunkHandle() override { return chunkHandle.lock(); }

    WeakChunkHandleType chunkHandle;
};

template<typename _Chunk>
struct ReleaseRequest:public ProcessRequest<_Chunk>
{
    ReleaseRequest(SharedChunkHandleType chunkHandle):ProcessRequest<_Chunk>(Process::Type::Release, 10, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle) {}
    ReleaseRequest(unsigned int priority, SharedChunkHandleType chunkHandle):ProcessRequest<_Chunk>(Type::Update, priority, chunkHandle->regionIndex(), chunkHandle->chunkIndex()), chunkHandle(chunkHandle) {}

    SharedChunkHandleType getChunkHandle() override { return chunkHandle.lock(); }

    WeakChunkHandleType chunkHandle;
};

//template<typename _Chunk>
//ChunkHandle<_Chunk> *getChunkHandleFromRequest(ProcessRequest<_Chunk> *request)
//{
//    ChunkHandle<_Chunk> *chunkHandle;
//
//    switch(request->type)
//    {
//    case Process::Type::Generate:
//        {
//            GenerateRequest<_Chunk> *generateRequest=(GenerateRequest<_Chunk> *)request;
//
//            chunkHandle=generateRequest->chunkHandle.lock().get();
//        }
//        break;
//    case Process::Type::Read:
//        {
//            ReadRequest<_Chunk> *readRequest=(ReadRequest<_Chunk> *)request;
//
//            chunkHandle=readRequest->chunkHandle.lock().get();
//        }
//        break;
//    case Process::Type::Write:
//        {
//            WriteRequest<_Chunk> *writeRequest=(WriteRequest<_Chunk> *)request;
//
//            chunkHandle=writeRequest->chunkHandle.get();
//        }
//        break;
//    case Process::Type::Update:
//        {
//            UpdateRequest<_Chunk> *updateRequest=(UpdateRequest<_Chunk> *)request;
//
//            chunkHandle=updateRequest->chunkHandle.lock().get();
//        }
//        break;
//    case Process::Type::Release:
//        {
//            ReleaseRequest<_Chunk> *releaseRequest=(UpdateRequest<_Chunk> *)request;
//
//            chunkHandle=releaseRequest->chunkHandle.lock().get();
//        }
//        break;
//    }
//
//    return chunkHandle;
//}

template<typename _Chunk>
struct ProcessCompare
{
    typedef _Chunk ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;

    bool operator()(const SharedProcessRequest<ChunkType> &request1, const SharedProcessRequest<ChunkType> &request2) const
    {
        if(request1->priority!=request2->priority)
            return (request1->priority>request2->priority);
        else
        {
            float handle1Distance=descriptor->getDistance(currentRegion, currentChunk, request1->regionIndex, request1->chunkIndex);
            float handle2Distance=descriptor->getDistance(currentRegion, currentChunk, request2->regionIndex, request2->chunkIndex);

#ifdef LOG_PROCESS_QUEUE
            request1->distance=handle1Distance;
            request2->distance=handle2Distance;
#endif
            return (handle1Distance>handle2Distance);
        }
    }

    static IGridDescriptors *descriptor;
    static glm::ivec3 currentRegion;
    static glm::ivec3 currentChunk;
};

template<typename _Chunk>
IGridDescriptors *ProcessCompare<_Chunk>::descriptor=nullptr;

template<typename _Chunk>
glm::ivec3 ProcessCompare<_Chunk>::currentRegion=glm::ivec3(0, 0, 0);

template<typename _Chunk>
glm::ivec3 ProcessCompare<_Chunk>::currentChunk=glm::ivec3(0, 0, 0);

template<typename _Grid>
class ProcessQueue
{
public:
    typedef typename _Grid::DescriptorType DescriptorType;

    typedef typename _Grid::ChunkType ChunkType;
    typedef ChunkHandle<ChunkType> ChunkHandleType;
    typedef std::shared_ptr<ChunkHandleType> SharedChunkHandle;
    typedef SharedProcessRequest<ChunkType> SharedRequest;

    typedef std::vector<SharedProcessRequest<ChunkType>> RequestQueue;

    typedef UpdateQueueRequest<ChunkType> UpdateQueueRequestType;
    typedef GenerateRequest<ChunkType> GenerateRequestType;
    typedef ReadRequest<ChunkType> ReadRequestType;
    typedef WriteRequest<ChunkType> WriteRequestType;
    typedef UpdateRequest<ChunkType> UpdateRequestType;
    typedef CancelRequest<ChunkType> CancelRequestType;
    typedef ReleaseRequest<ChunkType> ReleaseRequestType;

    ProcessQueue(DescriptorType *descriptor):
#ifndef NDEBUG
        m_inputThreadIdSet(false),
        m_outputThreadIdSet(false),
#endif
        m_checkForUpdates(0),
        m_descriptor(descriptor) 
    { ProcessCompare<ChunkType>::descriptor=descriptor; }

    void updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk);
    void addGenerate(SharedChunkHandle chunkHandle, size_t lod);
    void addRead(SharedChunkHandle chunkHandle, size_t lod);
    void addWrite(SharedChunkHandle chunkHandle);
    void addUpdate(SharedChunkHandle chunkHandle);
    void addCancel(SharedChunkHandle chunkHandle);
    void addRelease(SharedChunkHandle chunkHandle);

    bool empty() { return m_priorityQueue.empty(); }

    SharedRequest getNextProcessRequest(SharedChunkHandle &chunkHandle, size_t &data);
    void removeRequests(RequestQueue &cancelRequests);
    
    void updateQueue();
    void updatePriorityQueue();

    std::unique_lock<std::mutex> getLock();
    void trigger(std::unique_lock<std::mutex> &lock);
    void wait(std::unique_lock<std::mutex> &lock);

    void addCompletedRequest(SharedRequest request);
    void updateCompletedQueue();
    void getCompletedQueue(RequestQueue &completedQueue);
    bool isCompletedQueueEmpty();

//    void updatePriority();
private:
    void checkInputThread();
    void checkOutputThread();

#ifndef NDEBUG
    std::thread::id m_inputThreadId;
    bool m_inputThreadIdSet;
    std::thread::id m_outputThreadId;
    bool m_outputThreadIdSet;
#endif

    DescriptorType *m_descriptor;
    std::mutex m_processMutex;

    std::mutex m_queueMutex;
    std::condition_variable m_queueEvent;
//    std::priority_queue<SharedProcessRequest<ChunkType>> m_queue;
    RequestQueue m_cacheQueue;
    RequestQueue m_queue;
    RequestQueue m_priorityQueue;

    std::mutex m_completedMutex;
    RequestQueue m_completedQueueCache;
    RequestQueue m_outputCompletedQueue;
    RequestQueue m_completedQueue;

    std::atomic<int> m_checkForUpdates;
};


}//namespace voxigen

#include "processQueue.inl"

#endif //_voxigen_processQueue_h_